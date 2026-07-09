#include "updatemanager.h"
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QDir>
#include <QDirIterator>
#include <QProcess>
#include <QStorageInfo>
#include <QDebug>

Q_LOGGING_CATEGORY(lcUpdate, "carputer.update")

UpdateManager::UpdateManager(QObject *parent) : QObject(parent)
{
    m_nam = new QNetworkAccessManager(this);

    m_usbTimer = new QTimer(this);
    m_usbTimer->setInterval(USB_CHECK_MS);
    connect(m_usbTimer, &QTimer::timeout, this, &UpdateManager::scanUsbDrives);
    m_usbTimer->start();

    m_currentVersion = readCurrentVersion();
    setStatus(QStringLiteral("Ready"));
}

UpdateManager::~UpdateManager()
{
    if (m_reply) m_reply->abort();
    if (m_wget) {
        m_wget->kill();
        m_wget->waitForFinished(2000);
    }
}

QString UpdateManager::readCurrentVersion()
{
    QFile f{QLatin1String(VERSION_FILE)};
    if (f.open(QIODevice::ReadOnly))
        return QString::fromUtf8(f.readAll()).trimmed();
    return QStringLiteral("unknown");
}

void UpdateManager::checkForUpdate()
{
    if (m_busy) return;
    setBusy(true);
    setProgress(0);
    m_serverVersion.clear();
    m_downloadUrl.clear();
    m_updateAvailable = false;
    emit serverVersionChanged();
    emit updateAvailableChanged();

    setStatus(QStringLiteral("Checking for updates..."));

    QUrl apiUrl{QString::fromLatin1(GITHUB_API)};
    QNetworkRequest req{apiUrl};
    req.setRawHeader("Accept", "application/vnd.github.v3+json");
    req.setRawHeader("User-Agent", USER_AGENT);
    req.setTransferTimeout(15000);

    m_reply = m_nam->get(req);
    connect(m_reply, &QNetworkReply::finished,
            this, &UpdateManager::onGitHubReplyFinished);
}

void UpdateManager::onGitHubReplyFinished()
{
    if (!m_reply) return;

    if (m_reply->error() != QNetworkReply::NoError) {
        setStatus("Update check failed: " + m_reply->errorString());
        m_reply->deleteLater();
        m_reply = nullptr;
        setBusy(false);
        return;
    }

    QByteArray data = m_reply->readAll();
    m_reply->deleteLater();
    m_reply = nullptr;

    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(data, &err);
    if (err.error != QJsonParseError::NoError || !doc.isObject()) {
        setStatus(QStringLiteral("Update check failed: invalid response"));
        setBusy(false);
        return;
    }

    QJsonObject root = doc.object();

    QString tag = root.value("tag_name").toString();
    if (tag.isEmpty()) {
        setStatus(QStringLiteral("Update check failed: no version tag"));
        setBusy(false);
        return;
    }

    m_serverVersion = tag;
    if (m_serverVersion.startsWith('v'))
        m_serverVersion = m_serverVersion.mid(1);
    emit serverVersionChanged();

    QJsonArray assets = root.value("assets").toArray();
    for (const QJsonValue &val : assets) {
        QJsonObject asset = val.toObject();
        if (asset.value("name").toString() == QLatin1String(PACKAGE_NAME)) {
            m_downloadUrl = asset.value("browser_download_url").toString();
            break;
        }
    }

    if (m_downloadUrl.isEmpty()) {
        setStatus(QString("Version %1 found but no package asset").arg(m_serverVersion));
        setBusy(false);
        return;
    }

    m_updateAvailable = isNewerVersion(m_currentVersion, m_serverVersion);
    emit updateAvailableChanged();

    if (m_updateAvailable) {
        setStatus(QString("Update available: %1 \u2192 %2").arg(m_currentVersion, m_serverVersion));
    } else {
        setStatus(QString("Already up to date (%1)").arg(m_currentVersion));
    }

    setBusy(false);
}

void UpdateManager::applyNetworkUpdate()
{
    if (m_busy || m_downloadUrl.isEmpty()) return;
    setBusy(true);
    setProgress(0);
    setStatus(QStringLiteral("Downloading update..."));

    if (m_wget) {
        m_wget->kill();
        m_wget->waitForFinished(2000);
        m_wget->deleteLater();
    }
    m_wget = new QProcess(this);
    m_wget->setProcessChannelMode(QProcess::MergedChannels);
    connect(m_wget, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &UpdateManager::onWgetFinished);
    connect(m_wget, &QProcess::readyReadStandardOutput, this, [this]() {
        QString line = QString::fromUtf8(m_wget->readAllStandardOutput()).trimmed();
        if (!line.isEmpty())
            qDebug() << "[wget]" << line;
    });

    m_wget->start(QStringLiteral("wget"),
                  { QStringLiteral("-O"), QLatin1String(DOWNLOAD_TMP),
                    m_downloadUrl });
}

void UpdateManager::onWgetFinished()
{
    if (!m_wget) return;

    int code = m_wget->exitCode();
    QString output = QString::fromUtf8(m_wget->readAllStandardOutput()).trimmed();
    m_wget->deleteLater();
    m_wget = nullptr;

    if (code != 0) {
        setStatus(QString("Download failed (exit %1)").arg(code));
        if (!output.isEmpty())
            qCWarning(lcUpdate) << "wget:" << output;
        setBusy(false);
        QFile::remove(QLatin1String(DOWNLOAD_TMP));
        emit updateComplete(false, m_status);
        return;
    }

    setProgress(100);
    setStatus(QStringLiteral("Applying update..."));

    const bool ok = applyPackage(QLatin1String(DOWNLOAD_TMP));
    QFile::remove(QLatin1String(DOWNLOAD_TMP));

    if (ok) {
        m_currentVersion = readCurrentVersion();
        emit currentVersionChanged();
        setStatus(QString("Update applied! Now at %1 \u2014 restarting...").arg(m_currentVersion));
        emit updateComplete(true, m_status);
        QTimer::singleShot(2000, this, []() {
            QProcess::startDetached(QStringLiteral("systemctl"),
                                    { QStringLiteral("restart"), QStringLiteral("carputer") });
        });
    } else {
        setStatus(QStringLiteral("Update failed \u2014 check logs"));
        emit updateComplete(false, m_status);
    }

    setBusy(false);
}

void UpdateManager::scanUsbDrives()
{
    const QList<QStorageInfo> volumes = QStorageInfo::mountedVolumes();
    for (const QStorageInfo &vol : volumes) {
        if (!vol.isReady() || vol.isRoot()) continue;
        const QString mp = vol.rootPath();
        if (mp.startsWith("/proc") || mp.startsWith("/sys") || mp.startsWith("/dev"))
            continue;

        const QString pkg = mp + "/" + QLatin1String(PACKAGE_NAME);
        if (QFile::exists(pkg)) {
            if (m_usbPath != pkg) {
                m_usbPath        = pkg;
                m_usbUpdateFound = true;
                emit usbPathChanged();
                emit usbUpdateFoundChanged();
                setStatus(QString("USB update found: %1").arg(pkg));
                qCInfo(lcUpdate) << "USB package found at" << pkg;
            }
            return;
        }
    }

    if (m_usbUpdateFound) {
        m_usbPath        = QString();
        m_usbUpdateFound = false;
        emit usbPathChanged();
        emit usbUpdateFoundChanged();
        setStatus(QStringLiteral("Ready"));
    }
}

void UpdateManager::checkUsb()
{
    scanUsbDrives();
    if (!m_usbUpdateFound)
        setStatus(QStringLiteral("No update package found on USB drive"));
}

void UpdateManager::applyUsbUpdate()
{
    if (m_busy || m_usbPath.isEmpty() || !QFile::exists(m_usbPath)) {
        setStatus(QStringLiteral("No USB update package available"));
        return;
    }

    setBusy(true);
    setProgress(0);
    setStatus(QStringLiteral("Applying USB update..."));

    const bool ok = applyPackage(m_usbPath);

    if (ok) {
        m_currentVersion = readCurrentVersion();
        emit currentVersionChanged();
        setStatus(QString("Update applied! Now at %1 \u2014 restarting...").arg(m_currentVersion));
        emit updateComplete(true, m_status);
        QTimer::singleShot(2000, this, []() {
            QProcess::startDetached(QStringLiteral("systemctl"),
                                    { QStringLiteral("restart"), QStringLiteral("carputer") });
        });
    } else {
        setStatus(QStringLiteral("USB update failed \u2014 check logs"));
        emit updateComplete(false, m_status);
    }

    setBusy(false);
}

bool UpdateManager::applyPackage(const QString &packagePath)
{
    qCInfo(lcUpdate) << "Applying package:" << packagePath;

    const QString staging = QLatin1String(STAGING_DIR);
    QDir(staging).removeRecursively();
    if (!QDir().mkpath(staging)) {
        qCWarning(lcUpdate) << "Could not create staging dir:" << staging;
        return false;
    }

    QProcess tar;
    tar.start(QStringLiteral("sh"),
              { QStringLiteral("-c"),
                QStringLiteral("zcat '%1' | tar -xf - -C '%2'")
                    .arg(packagePath, staging) });
    if (!tar.waitForFinished(30000) || tar.exitCode() != 0) {
        qCWarning(lcUpdate) << "tar failed:" << tar.readAllStandardError();
        QDir(staging).removeRecursively();
        return false;
    }

    QString stagedBinary;
    QString stagedVersion;
    QDirIterator it(staging, QDir::Files | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        it.next();
        QString name = it.fileName();
        QString path = it.filePath();
        if (name == "carputer" && stagedBinary.isEmpty())
            stagedBinary = path;
        else if (name == "version.txt" && stagedVersion.isEmpty())
            stagedVersion = path;
    }

    const QString destBinary = QLatin1String(BINARY_DEST);
    if (stagedBinary.isEmpty() || !QFile::exists(stagedBinary)) {
        qCWarning(lcUpdate) << "Binary (carputer) not found in package";
        QDir(staging).removeRecursively();
        return false;
    }
    QFile::remove(destBinary);
    if (!QFile::copy(stagedBinary, destBinary)) {
        qCWarning(lcUpdate) << "Failed to copy binary to" << destBinary;
        QDir(staging).removeRecursively();
        return false;
    }
    QProcess::execute(QStringLiteral("chmod"), { QStringLiteral("+x"), destBinary });

    const QString destVersion = QLatin1String(VERSION_FILE);
    QDir().mkpath(QFileInfo(destVersion).path());
    if (!stagedVersion.isEmpty() && QFile::exists(stagedVersion)) {
        QFile::remove(destVersion);
        if (!QFile::copy(stagedVersion, destVersion)) {
            qCWarning(lcUpdate) << "Failed to install version.txt to" << destVersion;
        }
    } else {
        qCWarning(lcUpdate) << "version.txt not found in package";
    }

    QDir(staging).removeRecursively();

    qCInfo(lcUpdate) << "Package applied successfully";
    return true;
}

bool UpdateManager::isNewerVersion(const QString &current, const QString &latest)
{
    QString c = current;
    QString l = latest;
    if (c.startsWith('v')) c = c.mid(1);
    if (l.startsWith('v')) l = l.mid(1);

    QStringList cParts = c.split('.');
    QStringList lParts = l.split('.');

    int maxParts = qMax(cParts.size(), lParts.size());
    for (int i = 0; i < maxParts; ++i) {
        int cVal = (i < cParts.size()) ? cParts[i].toInt() : 0;
        int lVal = (i < lParts.size()) ? lParts[i].toInt() : 0;
        if (lVal > cVal) return true;
        if (lVal < cVal) return false;
    }
    return false;
}

void UpdateManager::setStatus(const QString &s)
{
    if (m_status == s) return;
    m_status = s;
    emit statusChanged();
    qCDebug(lcUpdate) << s;
}

void UpdateManager::setBusy(bool b)
{
    if (m_busy == b) return;
    m_busy = b;
    emit busyChanged();
}

void UpdateManager::setProgress(int p)
{
    if (m_progress == p) return;
    m_progress = p;
    emit progressChanged();
}
