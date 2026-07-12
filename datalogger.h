#ifndef DATALOGGER_H
#define DATALOGGER_H

#include <QObject>
#include <QFile>
#include <QTextStream>
#include <QTimer>
#include <QVariantList>
#include <QVariantMap>

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
    Q_INVOKABLE QVariantList listTrips();
    Q_INVOKABLE QVariantMap tripSummary(const QString &filePath);
    Q_INVOKABLE QVariantList tripData(const QString &filePath);

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
