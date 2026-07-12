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
             << "oilPressure,battery,fuel,brakeFluid,o2AFR,"
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
             << QString::number(m_sensorMgr->o2AFR(), 'f', 2) << ","
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

QVariantList DataLogger::listTrips()
{
    QVariantList trips;
    QDir dir(LOG_DIR);
    if (!dir.exists()) return trips;

    QStringList files = dir.entryList(QStringList() << "trip_*.csv", QDir::Files, QDir::Name | QDir::Reversed);
    for (const QString &file : files) {
        QVariantMap trip;
        QString fullPath = dir.absoluteFilePath(file);
        trip["path"] = fullPath;
        trip["name"] = file;

        QFile f(fullPath);
        if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) continue;

        QTextStream ts(&f);
        QString header = ts.readLine();
        int rowCount = 0;
        QString firstTs, lastTs;
        while (!ts.atEnd()) {
            QString line = ts.readLine();
            if (line.isEmpty()) continue;
            if (rowCount == 0) firstTs = line.section(',', 0, 0);
            lastTs = line.section(',', 0, 0);
            rowCount++;
        }
        f.close();

        trip["rows"] = rowCount;
        trip["firstTimestamp"] = firstTs;
        trip["lastTimestamp"] = lastTs;
        trips.append(trip);
    }
    return trips;
}

QVariantMap DataLogger::tripSummary(const QString &filePath)
{
    QVariantMap summary;
    QFile f(filePath);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) return summary;

    QTextStream ts(&f);
    QString header = ts.readLine();
    int rowCount = 0;
    double totalSpeed = 0, maxSpeed = 0;
    double totalRpm = 0, maxRpm = 0;
    double totalThrottle = 0, maxThrottle = 0;
    double totalMap = 0;
    double totalCoolant = 0, maxCoolant = 0;
    double totalOil = 0, maxOil = 0;
    double totalBattery = 0, minBattery = 99;
    double totalFuel = 0, maxFuel = 0;
    double totalO2 = 0;
    int o2Count = 0;
    QString firstTs, lastTs;

    while (!ts.atEnd()) {
        QString line = ts.readLine();
        if (line.isEmpty()) continue;
        QStringList cols = line.split(',');
        if (cols.size() < 14) continue;

        if (rowCount == 0) firstTs = cols[0];
        lastTs = cols[0];

        auto safeDouble = [](const QString &s, double def = 0) -> double {
            bool ok;
            double v = s.toDouble(&ok);
            return ok ? v : def;
        };

        double speed = safeDouble(cols[1]);
        double rpm = safeDouble(cols[2]);
        double throttle = safeDouble(cols[3]);
        double mapVal = safeDouble(cols[4]);
        double coolant = safeDouble(cols[5]);
        double oil = safeDouble(cols[6]);
        double battery = safeDouble(cols[10]);
        double fuel = safeDouble(cols[12]);
        double o2 = safeDouble(cols[14]);

        totalSpeed += speed;
        totalRpm += rpm;
        totalThrottle += throttle;
        totalMap += mapVal;
        totalCoolant += coolant;
        totalOil += oil;
        totalBattery += battery;
        totalFuel += fuel;
        if (o2 > 0) { totalO2 += o2; o2Count++; }

        if (speed > maxSpeed) maxSpeed = speed;
        if (rpm > maxRpm) maxRpm = rpm;
        if (throttle > maxThrottle) maxThrottle = throttle;
        if (coolant > maxCoolant) maxCoolant = coolant;
        if (oil > maxOil) maxOil = oil;
        if (battery < minBattery) minBattery = battery;
        if (fuel > maxFuel) maxFuel = fuel;

        rowCount++;
    }
    f.close();

    if (rowCount == 0) return summary;

    summary["rows"] = rowCount;
    summary["firstTimestamp"] = firstTs;
    summary["lastTimestamp"] = lastTs;
    summary["avgSpeed"] = totalSpeed / rowCount;
    summary["maxSpeed"] = maxSpeed;
    summary["avgRpm"] = totalRpm / rowCount;
    summary["maxRpm"] = maxRpm;
    summary["avgThrottle"] = totalThrottle / rowCount;
    summary["maxThrottle"] = maxThrottle;
    summary["maxCoolant"] = maxCoolant;
    summary["maxOil"] = maxOil;
    summary["minBattery"] = minBattery;
    summary["avgBattery"] = totalBattery / rowCount;
    summary["maxFuel"] = maxFuel;
    summary["avgO2AFR"] = o2Count > 0 ? totalO2 / o2Count : 0;

    return summary;
}

QVariantList DataLogger::tripData(const QString &filePath)
{
    QVariantList data;
    QFile f(filePath);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) return data;

    QTextStream ts(&f);
    ts.readLine(); // skip header

    while (!ts.atEnd()) {
        QString line = ts.readLine();
        if (line.isEmpty()) continue;
        QStringList cols = line.split(',');
        if (cols.size() < 14) continue;

        QVariantMap row;
        row["timestamp"] = cols[0];
        row["speed"] = cols[1].toDouble();
        row["rpm"] = cols[2].toDouble();
        row["throttle"] = cols[3].toDouble();
        row["map"] = cols[4].toDouble();
        row["coolant"] = cols[5].toDouble();
        row["oil"] = cols[6].toDouble();
        row["ambient"] = cols[7].toDouble();
        row["intake"] = cols[8].toDouble();
        row["oilPressure"] = cols[9].toDouble();
        row["battery"] = cols[10].toDouble();
        row["fuel"] = cols[12].toDouble();
        row["o2AFR"] = cols[14].toDouble();
        data.append(row);
    }
    f.close();
    return data;
}
