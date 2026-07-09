#ifndef DATALOGGER_H
#define DATALOGGER_H

#include <QObject>
#include <QFile>
#include <QTextStream>
#include <QTimer>

class SensorManager;

class DataLogger : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool logging READ logging NOTIFY loggingChanged)
    Q_PROPERTY(QString logPath READ logPath NOTIFY logPathChanged)

public:
    explicit DataLogger(SensorManager *sensorMgr, QObject *parent = nullptr);
    ~DataLogger() override;

    bool logging() const { return m_logging; }
    QString logPath() const { return m_logPath; }

    Q_INVOKABLE void startLogging();
    Q_INVOKABLE void stopLogging();

signals:
    void loggingChanged();
    void logPathChanged();

private:
    void writeHeader();
    void writeRow();
    void closeFile();

    SensorManager *m_sensorMgr;
    QFile m_file;
    QTextStream m_stream;
    QTimer *m_flushTimer;
    bool m_logging = false;
    QString m_logPath;
    int m_rowCount = 0;
};

#endif // DATALOGGER_H
