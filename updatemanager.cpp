#include "updatemanager.h"
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QFile>
#include <QDir>
#include <QProcess>
#include <QStorageInfo>
#include <QHostAddress>
#include <QNetworkDatagram>
#include <QDebug>

Q_LOGGING_CATEGORY(lcUpdate, "carputer.update")

// ── Constructor ───────────────────────────────────────────────────────────────

UpdateManager::UpdateManager(QObject *parent) : QObject(parent)
{
    m_nam = new QNetworkAccessManager(this);

    // UDP socket for LAN server auto-discovery
    m_udpSocket = new QUdpSocket(this);
    m_udpSocket->bind(QHostAddress::AnyIPv4, DISCOVERY_PORT,
                      QAbstractSocket::ShareAddress | QAbstractSocket::ReuseAddressHint);
    connect(m_udpSocket, &QUdpSocket::readyRead,
            this, &UpdateManager::onDiscoveryResponse);

    // USB drive scanner — checks every 3 s
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
}

// ── Version ───────────────────────────────────────────────────────────────────

QString UpdateManager::readCurrentVersion()
{
    QFile f{QLatin1String(VERSION_FILE)};
    if (f.open(QIODevice::ReadOnly))
        return QString::fromUtf8(f.readAll()).trimmed();
    return QStringLiteral("unknown");
}

// ── Server auto-discovery (UDP broadcast) ─────────────────────────────────────

void UpdateManager::discoverServer()
{
    setStatus(QStringLiteral("Searching for update server..."));
    m_udpSocket->writeDatagram("CARPUTER_UPDATE_DISCOVER",
                               QHostAddress::Broadcast, DISCOVERY_PORT);
    qCDebug(lcUpdate) << "Discovery broadcast sent";
}

void UpdateManager::onDiscoveryResponse()
{
    while (m_udpSocket->hasPendingDatagrams()) {
        const QNetworkDatagram dg = m_udpSocket->receiveDatagram();
        const QString response = QString::fromUtf8(dg.data()).trimmed();

        // Expected response: "CARPUTER_UPDATE_SERVER:<version>"
        if (!response.startsWith(QLatin1String("CARPUTER_UPDATE_SERVER:")))
            continue;

        m_serverVersion = response.section(':', 1);
        m_serverUrl     = QString("http://%1:%2").arg(dg.senderAddress().toString())
                                                 .arg(SERVER_PORT);

        qCInfo(lcUpdate) << "Update server found at" << m_serverUrl
                         << "version" << m_serverVersion;

        emit serverUrlChanged();
        emit serverVersionChanged();

        m_updateAvailable = (!m_serverVersion.isEmpty()
                             && m_serverVersion != m_currentVersion);
        emit updateAvailableChanged();

        setStatus(m_updateAvailable
            ? QString("Update available: %1 → %2").arg(m_currentVersion, m_serverVersion)
            : QString("Already up to date (%1)").arg(m_currentVersion));

        setBusy(false);
    }
}

// ── Network update ────────────────────────────────────────────────────────────

void UpdateManager::checkForUpdate()
{
    if (m_busy) return;
    setBusy(true);
    setProgress(0);
    m_serverUrl.clear();
    m_serverVersion.clear();
    m_updateAvailable = false;
    emit serverUrlChanged();
    emit serverVersionChanged();
    emit updateAvailableChanged();

    discoverServer();

    // Time out after 5 s if no server responds
    QTimer::singleShot(5000, this, [this]() {
        if (m_serverUrl.isEmpty()) {
            setBusy(false);
            setStatus(QStringLiteral("No update server found on network"));
        }
    });
}

void UpdateManager::applyNetworkUpdate()
{
    if (m_busy || m_serverUrl.isEmpty()) return;
    setBusy(true);
    setProgress(0);
    setStatus(QStringLiteral("Downloading update..."));

    const QString url = m_serverUrl + "/" + QLatin1String(PACKAGE_NAME);
    QNetworkRequest req(url);
    m_reply = m_nam->get(req);

    connect(m_reply, &QNetworkReply::downloadProgress,
            this, &UpdateManager::onDownloadProgress);
    connect(m_reply, &QNetworkReply::finished,
            this, &UpdateManager::onDownloadFinished);
}

void UpdateManager::onDownloadProgress(qint64 received, qint64 total)
{
    if (total > 0) {
        setProgress(static_cast<int>(received * 100 / total));
        setStatus(QString("Downloading... %1%").arg(m_progress));
    }
}

void UpdateManager::onDownloadFinished()
{
    if (!m_reply) return;

    if (m_reply->error() != QNetworkReply::NoError) {
        setStatus("Download failed: " + m_reply->errorString());
        setBusy(false);
        m_reply->deleteLater();
        m_reply = nullptr;
        emit updateComplete(false, m_status);
        return;
    }

    // Write download to temp file
    QFile f{QLatin1String(DOWNLOAD_TMP)};
    if (!f.open(QIODevice::WriteOnly)) {
        setStatus(QStringLiteral("Failed to write download to /tmp"));
        setBusy(false);
        m_reply->deleteLater();
        m_reply = nullptr;
        emit updateComplete(false, m_status);
        return;
    }
    f.write(m_reply->readAll());
    f.close();
    m_reply->deleteLater();
    m_reply = nullptr;

    setProgress(100);
    setStatus(QStringLiteral("Applying update..."));

    const bool ok = applyPackage(QLatin1String(DOWNLOAD_TMP));
    QFile::remove(QLatin1String(DOWNLOAD_TMP));

    if (ok) {
        m_currentVersion = readCurrentVersion();
        emit currentVersionChanged();
        setStatus(QString("Update applied! Now at %1 — restarting...").arg(m_currentVersion));
        emit updateComplete(true, m_status);
        QTimer::singleShot(2000, this, []() {
            QProcess::startDetached(QStringLiteral("systemctl"),
                                    { QStringLiteral("restart"), QStringLiteral("carputer") });
        });
    } else {
        setStatus(QStringLiteral("Update failed — check logs"));
        emit updateComplete(false, m_status);
    }

    setBusy(false);
}

// ── USB update ────────────────────────────────────────────────────────────────

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
        setStatus(QString("Update applied! Now at %1 — restarting...").arg(m_currentVersion));
        emit updateComplete(true, m_status);
        QTimer::singleShot(2000, this, []() {
            QProcess::startDetached(QStringLiteral("systemctl"),
                                    { QStringLiteral("restart"), QStringLiteral("carputer") });
        });
    } else {
        setStatus(QStringLiteral("USB update failed — check logs"));
        emit updateComplete(false, m_status);
    }

    setBusy(false);
}

// ── Package extraction ────────────────────────────────────────────────────────
//
// Bug fix: previously the tar was extracted directly to /usr/bin/ which put
// version.txt at /usr/bin/version.txt. The app reads version from
// /etc/carputer/version.txt, so the version never updated after OTA installs.
//
// Fix: extract to a temp staging dir, then copy each file to its proper dest:
//   carputer    → /usr/bin/carputer
//   version.txt → /etc/carputer/version.txt
//
bool UpdateManager::applyPackage(const QString &packagePath)
{
    qCInfo(lcUpdate) << "Applying package:" << packagePath;

    // Clean and create staging dir
    const QString staging = QLatin1String(STAGING_DIR);
    QDir(staging).removeRecursively();
    if (!QDir().mkpath(staging)) {
        qCWarning(lcUpdate) << "Could not create staging dir:" << staging;
        return false;
    }

    // Extract package into staging dir
    QProcess tar;
    tar.start(QStringLiteral("tar"),
              { QStringLiteral("-xzf"), packagePath,
                QStringLiteral("-C"),   staging });
    if (!tar.waitForFinished(30000) || tar.exitCode() != 0) {
        qCWarning(lcUpdate) << "tar failed:" << tar.readAllStandardError();
        QDir(staging).removeRecursively();
        return false;
    }

    // ── Install binary ─────────────────────────────────────────────────────
    const QString stagedBinary = staging + "/carputer";
    const QString destBinary   = QLatin1String(BINARY_DEST);
    if (!QFile::exists(stagedBinary)) {
        qCWarning(lcUpdate) << "Binary not found in package";
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

    // ── Install version.txt → /etc/carputer/version.txt ───────────────────
    const QString stagedVersion = staging + "/version.txt";
    const QString destVersion   = QLatin1String(VERSION_FILE);
    QDir().mkpath(QFileInfo(destVersion).path()); // ensure /etc/carputer/ exists
    if (QFile::exists(stagedVersion)) {
        QFile::remove(destVersion);
        if (!QFile::copy(stagedVersion, destVersion)) {
            qCWarning(lcUpdate) << "Failed to install version.txt to" << destVersion;
            // Non-fatal — binary is already installed
        }
    } else {
        qCWarning(lcUpdate) << "version.txt missing from package — version will not update";
    }

    // Cleanup staging
    QDir(staging).removeRecursively();

    qCInfo(lcUpdate) << "Package applied successfully";
    return true;
}

// ── Helpers ───────────────────────────────────────────────────────────────────

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
