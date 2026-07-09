#ifndef TRIPCOMPUTER_H
#define TRIPCOMPUTER_H

#include <QObject>
#include <QElapsedTimer>
#include <QTimer>
#include <QJsonObject>

class SensorManager;
class EngineProfileManager;

class TripComputer : public QObject
{
    Q_OBJECT
    Q_PROPERTY(double distance READ distance NOTIFY distanceChanged)
    Q_PROPERTY(double avgSpeed READ avgSpeed NOTIFY avgSpeedChanged)
    Q_PROPERTY(double fuelUsed READ fuelUsed NOTIFY fuelUsedChanged)
    Q_PROPERTY(double instantMpg READ instantMpg NOTIFY instantMpgChanged)
    Q_PROPERTY(int tripTime READ tripTime NOTIFY tripTimeChanged)
    Q_PROPERTY(bool running READ running NOTIFY runningChanged)

public:
    explicit TripComputer(SensorManager *sensorMgr, QObject *parent = nullptr);
    ~TripComputer() override;

    double distance() const { return m_distance; }
    double avgSpeed() const { return m_avgSpeed; }
    double fuelUsed() const { return m_fuelUsed; }
    double instantMpg() const { return m_instantMpg; }
    int tripTime() const { return m_tripTime; }
    bool running() const { return m_running; }

    Q_INVOKABLE void reset();
    Q_INVOKABLE void start();
    Q_INVOKABLE void stop();
    void setEngineProfileManager(EngineProfileManager *mgr);

signals:
    void distanceChanged();
    void avgSpeedChanged();
    void fuelUsedChanged();
    void instantMpgChanged();
    void tripTimeChanged();
    void runningChanged();

private slots:
    void onTick();

private:
    void load();
    void save();
    void setDistance(double v);
    void setAvgSpeed(double v);
    void setFuelUsed(double v);
    void setInstantMpg(double v);
    void setTripTime(int v);
    void setRunning(bool v);

    SensorManager *m_sensorMgr;
    QTimer *m_tickTimer;
    QElapsedTimer m_elapsed;

    double m_distance = 0.0;
    double m_avgSpeed = 0.0;
    double m_fuelUsed = 0.0;
    double m_instantMpg = 0.0;
    int m_tripTime = 0;
    bool m_running = false;

    int m_lastSpeed = 0;
    int m_lastRpm = 0;
    int m_lastThrottle = 0;
    EngineProfileManager *m_engineProfile = nullptr;
};

#endif // TRIPCOMPUTER_H
