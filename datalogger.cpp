#include "datalogger.h"
#include "sensormanager.h"
#include <QDir>
#include <QDateTime>
#include <QDebug>

static const QString LOG_DIR = QStringLiteral("/root/carputer/logs");

DataLogger::DataLogger(SensorManager *sensorMgr, QObject *parent)
    : QObject(parent), m_sensorMgr(sensorMgr)
{
    connect(sensorMgr, &SensorManager::sensorDataReceived, this, [this]() {
        if (m_logging) writeRow();
    });

    m_flushTimer = new QTimer(this);
    m_flushTimer->setInterval(5000);
    connect(m_flushTimer, &QTimer::timeout, this, [this]() {
        if (m_logging) {
            m_stream.flush();
        }
    });
}

DataLogger::~DataLogger()
{
    stopLogging();
}

void DataLogger::startLogging()
{
    if (m_logging) return;

    QDir().mkpath(LOG_DIR);
    QString ts = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss");
    m_logPath = LOG_DIR + "/trip_" + ts + ".csv";
    m_file.setFileName(m_logPath);

    if (!m_file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "DataLogger: failed to open" << m_logPath;
        return;
    }

    m_stream.setDevice(&m_file);
    writeHeader();
    m_logging = true;
    m_rowCount = 0;
    m_flushTimer->start();
    emit loggingChanged();
    emit logPathChanged();
    qDebug() << "DataLogger: started logging to" << m_logPath;
}

void DataLogger::stopLogging()
{
    if (!m_logging) return;
    m_flushTimer->stop();
    m_stream.flush();
    closeFile();
    m_logging = false;
    emit loggingChanged();
    qDebug() << "DataLogger: stopped, wrote" << m_rowCount << "rows to" << m_logPath;
}

void DataLogger::writeHeader()
{
    m_stream << "timestamp,speed,rpm,throttle,map,coolant,oil,ambient,intake,"
             << "oilPressure,battery,fuel,brakeFluid,"
             << "driverDoor,passengerDoor,rearLeftDoor,rearRightDoor,trunk,hood\n";
}

void DataLogger::writeRow()
{
    QString ts = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
    m_stream << ts << ","
             << m_sensorMgr->speed() << ","
             << m_sensorMgr->rpm() << ","
             << m_sensorMgr->throttle() << ","
             << m_sensorMgr->map() << ","
             << m_sensorMgr->coolantTemp() << ","
             << m_sensorMgr->oilTemp() << ","
             << m_sensorMgr->ambientTemp() << ","
             << m_sensorMgr->intakeTemp() << ","
             << m_sensorMgr->oilPressure() << ","
             << m_sensorMgr->battery() << ","
             << m_sensorMgr->fuelLevel() << ","
             << m_sensorMgr->brakeFluid() << ","
             << (m_sensorMgr->driverDoor() ? "1" : "0") << ","
             << (m_sensorMgr->passengerDoor() ? "1" : "0") << ","
             << (m_sensorMgr->rearLeftDoor() ? "1" : "0") << ","
             << (m_sensorMgr->rearRightDoor() ? "1" : "0") << ","
             << (m_sensorMgr->trunk() ? "1" : "0") << ","
             << (m_sensorMgr->hood() ? "1" : "0") << "\n";
    m_rowCount++;
}

void DataLogger::closeFile()
{
    if (m_file.isOpen())
        m_file.close();
}
