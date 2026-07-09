#include "tripcomputer.h"
#include "sensormanager.h"
#include "engineprofilemanager.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDir>
#include <QDebug>
#include <QtMath>

static const QString TRIP_FILE = QStringLiteral("/root/carputer/trip.json");

TripComputer::TripComputer(SensorManager *sensorMgr, QObject *parent)
    : QObject(parent), m_sensorMgr(sensorMgr)
{
    load();

    connect(sensorMgr, &SensorManager::speedChanged, this, [this]() {
        m_lastSpeed = m_sensorMgr->speed();
    });
    connect(sensorMgr, &SensorManager::rpmChanged, this, [this]() {
        m_lastRpm = m_sensorMgr->rpm();
    });
    connect(sensorMgr, &SensorManager::throttleChanged, this, [this]() {
        m_lastThrottle = m_sensorMgr->throttle();
    });

    m_tickTimer = new QTimer(this);
    m_tickTimer->setInterval(1000);
    connect(m_tickTimer, &QTimer::timeout, this, &TripComputer::onTick);
}

TripComputer::~TripComputer()
{
    if (m_running) save();
}

void TripComputer::start()
{
    if (m_running) return;
    m_elapsed.start();
    setRunning(true);
    m_tickTimer->start();
}

void TripComputer::stop()
{
    if (!m_running) return;
    m_tickTimer->stop();
    setRunning(false);
    save();
}

void TripComputer::reset()
{
    m_tickTimer->stop();
    setDistance(0.0);
    setAvgSpeed(0.0);
    setFuelUsed(0.0);
    setInstantMpg(0.0);
    setTripTime(0);
    m_lastSpeed = 0;
    m_lastRpm = 0;
    m_lastThrottle = 0;
    setRunning(false);
    QFile::remove(TRIP_FILE);
}

void TripComputer::onTick()
{
    if (!m_running) return;

    int elapsed = m_elapsed.restart();
    double hours = elapsed / 3600000.0;

    // Distance: speed (mph) * time (hours)
    double distDelta = m_lastSpeed * hours;
    setDistance(m_distance + distDelta);

    // Trip time
    setTripTime(m_tripTime + 1);

    // Average speed
    if (m_tripTime > 0)
        setAvgSpeed(m_distance / (m_tripTime / 3600.0));

    // Fuel estimate: simple model based on throttle+RPM
    // At idle (throttle < 5%): ~0.2 gal/hr
    // At cruise (throttle 5-25%): ~0.5-1.5 gal/hr
    // At WOT (throttle > 80%): ~8+ gal/hr
    double throttleFactor = qBound(0.0, m_lastThrottle / 100.0, 1.0);
    int normRPM = m_engineProfile ? m_engineProfile->redlineRPM() : 6000;
    double rpmFactor = qBound(0.0, static_cast<double>(m_lastRpm) / normRPM, 1.0);
    double fuelRate = 0.2 + (throttleFactor * rpmFactor * 8.0);
    double fuelDelta = fuelRate * hours;
    setFuelUsed(m_fuelUsed + fuelDelta);

    // Instant MPG
    double instMpg = 0.0;
    if (fuelDelta > 0.0 && distDelta > 0.0)
        instMpg = distDelta / fuelDelta;
    else if (m_lastRpm > 0 && m_lastSpeed > 0)
        instMpg = 60.0; // coasting estimate
    setInstantMpg(instMpg);

    // Persist every 15 seconds
    if (m_tripTime % 15 == 0)
        save();
}

void TripComputer::load()
{
    QFile file(TRIP_FILE);
    if (!file.open(QIODevice::ReadOnly))
        return;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();
    if (!doc.isObject()) return;
    QJsonObject obj = doc.object();
    m_distance = obj.value("distance").toDouble();
    m_fuelUsed = obj.value("fuelUsed").toDouble();
    m_tripTime = obj.value("tripTime").toInt();
    if (m_tripTime > 0)
        m_avgSpeed = m_distance / (m_tripTime / 3600.0);
}

void TripComputer::save()
{
    QDir().mkpath(QStringLiteral("/root/carputer"));
    QFile file(TRIP_FILE);
    if (!file.open(QIODevice::WriteOnly))
        return;
    QJsonObject obj;
    obj["distance"] = m_distance;
    obj["fuelUsed"] = m_fuelUsed;
    obj["tripTime"] = m_tripTime;
    file.write(QJsonDocument(obj).toJson());
    file.close();
}

void TripComputer::setDistance(double v)
{
    if (qFuzzyCompare(m_distance, v)) return;
    m_distance = v;
    emit distanceChanged();
}

void TripComputer::setAvgSpeed(double v)
{
    if (qFuzzyCompare(m_avgSpeed, v)) return;
    m_avgSpeed = v;
    emit avgSpeedChanged();
}

void TripComputer::setFuelUsed(double v)
{
    if (qFuzzyCompare(m_fuelUsed, v)) return;
    m_fuelUsed = v;
    emit fuelUsedChanged();
}

void TripComputer::setInstantMpg(double v)
{
    if (qFuzzyCompare(m_instantMpg, v)) return;
    m_instantMpg = v;
    emit instantMpgChanged();
}

void TripComputer::setTripTime(int v)
{
    if (m_tripTime == v) return;
    m_tripTime = v;
    emit tripTimeChanged();
}

void TripComputer::setRunning(bool v)
{
    if (m_running == v) return;
    m_running = v;
    emit runningChanged();
}

void TripComputer::setEngineProfileManager(EngineProfileManager *mgr)
{
    m_engineProfile = mgr;
}
