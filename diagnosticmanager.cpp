#include "diagnosticmanager.h"

#include <QDateTime>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMutexLocker>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(lcDiag, "carputer.diag")

std::atomic<DiagnosticManager *> DiagnosticManager::s_instance{nullptr};

DiagnosticManager::DiagnosticManager(QObject *parent)
    : QObject(parent)
{
    s_instance.store(this, std::memory_order_relaxed);

    m_snapshotTimer.setInterval(2000);
    connect(&m_snapshotTimer, &QTimer::timeout,
            this,             &DiagnosticManager::writeSnapshot);
}

DiagnosticManager::~DiagnosticManager()
{
    qInstallMessageHandler(nullptr);
    s_instance.store(nullptr, std::memory_order_release);
    stopLogging();
}

DiagnosticManager *DiagnosticManager::instance()
{
    return s_instance.load(std::memory_order_relaxed);
}

bool DiagnosticManager::isLogging() const
{
    return m_logging;
}

QString DiagnosticManager::logFilePath() const
{
    return m_logFilePath;
}

void DiagnosticManager::appendLogLine(const QString &line)
{
    QMutexLocker lock(&m_mutex);
    if (m_logFile.isOpen()) {
        m_logFile.write((line + QLatin1Char('\n')).toUtf8());
        m_logFile.flush();
    }
}

void DiagnosticManager::startLogging()
{
    if (m_logging)
        return;

    const QString ts   = QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMdd_HHmmss"));
    const QString path = QStringLiteral("/tmp/carputer_diag_%1.log").arg(ts);

    openLogFile(path);
    if (!m_logFile.isOpen()) {
        qCWarning(lcDiag) << "Could not open diagnostic log file:" << path;
        return;
    }

    m_logging = true;
    m_snapshotTimer.start();
    qCInfo(lcDiag) << "Diagnostic logging started:" << path;
    emit loggingChanged();
    emit logFilePathChanged();
}

void DiagnosticManager::stopLogging()
{
    if (!m_logging)
        return;

    m_snapshotTimer.stop();

    {
        QMutexLocker lock(&m_mutex);
        if (m_logFile.isOpen()) {
            m_logFile.write(
                QStringLiteral("# --- Diagnostic logging stopped at %1 ---\n")
                    .arg(QDateTime::currentDateTime().toString(Qt::ISODate))
                    .toUtf8());
            m_logFile.flush();
        }
    }

    closeLogFile();
    m_logging = false;
    qCInfo(lcDiag) << "Diagnostic logging stopped";
    emit loggingChanged();
}

QString DiagnosticManager::exportReport()
{
    const bool wasLogging = m_logging;

    if (!wasLogging)
        startLogging();

    writeSnapshot();

    const QString path = m_logFilePath;

    if (!wasLogging)
        stopLogging();

    return path;
}

void DiagnosticManager::writeSnapshot()
{
    const QString json = buildSnapshotJson();
    QMutexLocker lock(&m_mutex);
    if (m_logFile.isOpen()) {
        m_logFile.write((json + QLatin1Char('\n')).toUtf8());
        m_logFile.flush();
    }
}

void DiagnosticManager::openLogFile(const QString &path)
{
    QMutexLocker lock(&m_mutex);
    m_logFile.setFileName(path);
    if (m_logFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        m_logFilePath = path;
        m_logFile.write(
            QStringLiteral("# Carputer Diagnostic Log\n"
                           "# Started: %1\n"
                           "# --- logs follow ---\n")
                .arg(QDateTime::currentDateTime().toString(Qt::ISODate))
                .toUtf8());
        m_logFile.flush();
    }
}

void DiagnosticManager::closeLogFile()
{
    QMutexLocker lock(&m_mutex);
    if (m_logFile.isOpen())
        m_logFile.close();
}

QString DiagnosticManager::buildSnapshotJson() const
{
    QJsonObject root;
    root[QLatin1String("ts")]   = QDateTime::currentDateTime().toString(Qt::ISODate);
    root[QLatin1String("type")] = QLatin1String("snapshot");

    return QString::fromUtf8(QJsonDocument(root).toJson(QJsonDocument::Compact));
}