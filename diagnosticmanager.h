#pragma once
#include <QObject>
#include <QFile>
#include <QTimer>
#include <QMutex>
#include <atomic>

// ── DiagnosticManager ──────────────────────────────────────────────────────────
// Collects runtime data and writes to a timestamped log file in /tmp.
// Two modes:
//
//   • Continuous logging  — call startLogging() / stopLogging().  A 2-second
//     timer writes compact JSON snapshots of all key state to the file.
//
//   • One-shot export     — call exportReport().  Writes one snapshot immediately.
//
// NOTE: This class must be constructed and destroyed on the main thread.
//       appendLogLine() is thread-safe and may be called from any thread.
// ─────────────────────────────────────────────────────────────────────────────

class DiagnosticManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool    logging     READ isLogging     NOTIFY loggingChanged)
    Q_PROPERTY(QString logFilePath READ logFilePath   NOTIFY logFilePathChanged)

public:
    explicit DiagnosticManager(QObject *parent = nullptr);
    ~DiagnosticManager();

    static DiagnosticManager *instance();

    bool    isLogging()   const;
    QString logFilePath() const;

    void appendLogLine(const QString &line);

public slots:
    Q_INVOKABLE void startLogging();
    Q_INVOKABLE void stopLogging();
    Q_INVOKABLE QString exportReport();

signals:
    void loggingChanged();
    void logFilePathChanged();

private slots:
    void writeSnapshot();

private:
    void    openLogFile(const QString &path);
    void    closeLogFile();
    QString buildSnapshotJson() const;

    static std::atomic<DiagnosticManager *> s_instance;

    QTimer       m_snapshotTimer;
    QFile        m_logFile;
    mutable QMutex m_mutex;
    QString      m_logFilePath;
    bool         m_logging = false;
};