#ifndef SENSOMANAGER_H
#define SENSOMANAGER_H

#include <QObject>
#include <QUdpSocket>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTimer>

class SensorManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool connected READ connected NOTIFY connectedChanged)
    Q_PROPERTY(QString statusText READ statusText NOTIFY statusTextChanged)

    // Vehicle dynamics
    Q_PROPERTY(int speed READ speed NOTIFY speedChanged)
    Q_PROPERTY(int rpm READ rpm NOTIFY rpmChanged)
    Q_PROPERTY(int throttle READ throttle NOTIFY throttleChanged)
    Q_PROPERTY(int map READ map NOTIFY mapChanged)

    // Temperature sensors (°F)
    Q_PROPERTY(int coolantTemp READ coolantTemp NOTIFY coolantTempChanged)
    Q_PROPERTY(int oilTemp READ oilTemp NOTIFY oilTempChanged)
    Q_PROPERTY(int ambientTemp READ ambientTemp NOTIFY ambientTempChanged)
    Q_PROPERTY(int intakeTemp READ intakeTemp NOTIFY intakeTempChanged)

    // Door sensors
    Q_PROPERTY(bool driverDoor READ driverDoor NOTIFY driverDoorChanged)
    Q_PROPERTY(bool passengerDoor READ passengerDoor NOTIFY passengerDoorChanged)
    Q_PROPERTY(bool rearLeftDoor READ rearLeftDoor NOTIFY rearLeftDoorChanged)
    Q_PROPERTY(bool rearRightDoor READ rearRightDoor NOTIFY rearRightDoorChanged)
    Q_PROPERTY(bool trunk READ trunk NOTIFY trunkChanged)
    Q_PROPERTY(bool hood READ hood NOTIFY hoodChanged)

    // Analog sensors (%)
    Q_PROPERTY(int fuelLevel READ fuelLevel NOTIFY fuelLevelChanged)
    Q_PROPERTY(int oilPressure READ oilPressure NOTIFY oilPressureChanged)
    Q_PROPERTY(int brakeFluid READ brakeFluid NOTIFY brakeFluidChanged)
    Q_PROPERTY(int battery READ battery NOTIFY batteryChanged)

    // Wideband O2 (piggyback module)
    Q_PROPERTY(double wboAFR READ wboAFR NOTIFY wboAFRChanged)
    Q_PROPERTY(double wboLambda READ wboLambda NOTIFY wboLambdaChanged)
    Q_PROPERTY(double targetAFR READ targetAFR NOTIFY targetAFRChanged)
    Q_PROPERTY(double fuelCorrection READ fuelCorrection NOTIFY fuelCorrectionChanged)

    // Narrowband O2 (direct from sensor module)
    Q_PROPERTY(double o2AFR READ o2AFR NOTIFY o2AFRChanged)

    // OBD2 data (from sensor module, echoed from ELM327 via serial bridge)
    Q_PROPERTY(int obd2Rpm READ obd2Rpm NOTIFY obd2RpmChanged)
    Q_PROPERTY(int obd2Speed READ obd2Speed NOTIFY obd2SpeedChanged)
    Q_PROPERTY(int obd2Load READ obd2Load NOTIFY obd2LoadChanged)
    Q_PROPERTY(int obd2Coolant READ obd2Coolant NOTIFY obd2CoolantChanged)
    Q_PROPERTY(int obd2Intake READ obd2Intake NOTIFY obd2IntakeChanged)
    Q_PROPERTY(int obd2Timing READ obd2Timing NOTIFY obd2TimingChanged)
    Q_PROPERTY(int obd2FuelTrimShort READ obd2FuelTrimShort NOTIFY obd2FuelTrimShortChanged)
    Q_PROPERTY(int obd2FuelTrimLong READ obd2FuelTrimLong NOTIFY obd2FuelTrimLongChanged)
    Q_PROPERTY(int obd2FuelPressure READ obd2FuelPressure NOTIFY obd2FuelPressureChanged)
    Q_PROPERTY(int obd2DistanceDtc READ obd2DistanceDtc NOTIFY obd2DistanceDtcChanged)

public:
    explicit SensorManager(QObject *parent = nullptr);
    ~SensorManager() override;

    bool connected() const { return m_connected; }
    QString statusText() const { return m_statusText; }

    int speed() const { return m_speed; }
    int rpm() const { return m_rpm; }
    int throttle() const { return m_throttle; }
    int map() const { return m_map; }
    int coolantTemp() const { return m_coolantTemp; }
    int oilTemp() const { return m_oilTemp; }
    int ambientTemp() const { return m_ambientTemp; }
    int intakeTemp() const { return m_intakeTemp; }

    bool driverDoor() const { return m_driverDoor; }
    bool passengerDoor() const { return m_passengerDoor; }
    bool rearLeftDoor() const { return m_rearLeftDoor; }
    bool rearRightDoor() const { return m_rearRightDoor; }
    bool trunk() const { return m_trunk; }
    bool hood() const { return m_hood; }

    int fuelLevel() const { return m_fuelLevel; }
    int oilPressure() const { return m_oilPressure; }
    int brakeFluid() const { return m_brakeFluid; }
    int battery() const { return m_battery; }

    // Wideband O2
    double wboAFR() const { return m_wboAFR; }
    double wboLambda() const { return m_wboLambda; }
    double targetAFR() const { return m_targetAFR; }
    double fuelCorrection() const { return m_fuelCorrection; }

    // Narrowband O2
    double o2AFR() const { return m_o2AFR; }

    // OBD2
    int obd2Rpm() const { return m_obd2Rpm; }
    int obd2Speed() const { return m_obd2Speed; }
    int obd2Load() const { return m_obd2Load; }
    int obd2Coolant() const { return m_obd2Coolant; }
    int obd2Intake() const { return m_obd2Intake; }
    int obd2Timing() const { return m_obd2Timing; }
    int obd2FuelTrimShort() const { return m_obd2FuelTrimShort; }
    int obd2FuelTrimLong() const { return m_obd2FuelTrimLong; }
    int obd2FuelPressure() const { return m_obd2FuelPressure; }
    int obd2DistanceDtc() const { return m_obd2DistanceDtc; }

public slots:
    void reconnect();

signals:
    void connectedChanged();
    void statusTextChanged();
    void speedChanged();
    void rpmChanged();
    void throttleChanged();
    void mapChanged();
    void coolantTempChanged();
    void oilTempChanged();
    void ambientTempChanged();
    void intakeTempChanged();
    void driverDoorChanged();
    void passengerDoorChanged();
    void rearLeftDoorChanged();
    void rearRightDoorChanged();
    void trunkChanged();
    void hoodChanged();
    void fuelLevelChanged();
    void oilPressureChanged();
    void brakeFluidChanged();
    void batteryChanged();
    void sensorDataReceived(const QString &data);

    // Wideband O2
    void wboAFRChanged();
    void wboLambdaChanged();
    void targetAFRChanged();
    void fuelCorrectionChanged();

    // Narrowband O2
    void o2AFRChanged();

    // OBD2
    void obd2RpmChanged();
    void obd2SpeedChanged();
    void obd2LoadChanged();
    void obd2CoolantChanged();
    void obd2IntakeChanged();
    void obd2TimingChanged();
    void obd2FuelTrimShortChanged();
    void obd2FuelTrimLongChanged();
    void obd2FuelPressureChanged();
    void obd2DistanceDtcChanged();

private slots:
    void onReadyRead();

private:
    void parseSensorJson(const QByteArray &data);
    void setConnected(bool c);
    void setStatusText(const QString &t);

    QUdpSocket m_socket;
    bool m_connected = false;
    QString m_statusText;
    QTimer *m_activityTimer = nullptr;

    int m_speed = 0;
    int m_rpm = 0;
    int m_throttle = 0;
    int m_map = 0;
    int m_coolantTemp = 0;
    int m_oilTemp = 0;
    int m_ambientTemp = 0;
    int m_intakeTemp = 0;

    bool m_driverDoor = false;
    bool m_passengerDoor = false;
    bool m_rearLeftDoor = false;
    bool m_rearRightDoor = false;
    bool m_trunk = false;
    bool m_hood = false;

    int m_fuelLevel = 0;
    int m_oilPressure = 0;
    int m_brakeFluid = 0;
    int m_battery = 0;

    // Wideband O2
    double m_wboAFR = 14.7;
    double m_wboLambda = 1.0;
    double m_targetAFR = 14.7;
    double m_fuelCorrection = 0.0;

    // Narrowband O2
    double m_o2AFR = 0.0;

    // OBD2
    int m_obd2Rpm = 0;
    int m_obd2Speed = 0;
    int m_obd2Load = 0;
    int m_obd2Coolant = 0;
    int m_obd2Intake = 0;
    int m_obd2Timing = 0;
    int m_obd2FuelTrimShort = 0;
    int m_obd2FuelTrimLong = 0;
    int m_obd2FuelPressure = 0;
    int m_obd2DistanceDtc = 0;

    QByteArray m_buffer;
};

#endif // SENSOMANAGER_H
