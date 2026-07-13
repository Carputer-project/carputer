#ifndef BLUETOOTHMANAGER_H
#define BLUETOOTHMANAGER_H

#include <QObject>
#include <QUdpSocket>
#include <QTimer>
#include <QStringList>

class BluetoothManager : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool connected READ connected NOTIFY connectedChanged)
    Q_PROPERTY(QString statusText READ statusText NOTIFY statusTextChanged)
    Q_PROPERTY(QString deviceName READ deviceName NOTIFY deviceNameChanged)
    Q_PROPERTY(QString deviceAddress READ deviceAddress NOTIFY deviceAddressChanged)

    // OBD2 live data
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
    explicit BluetoothManager(QObject *parent = nullptr);
    ~BluetoothManager() override;

    bool connected() const { return m_connected; }
    QString statusText() const { return m_statusText; }
    QString deviceName() const { return m_deviceName; }
    QString deviceAddress() const { return m_deviceAddress; }

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

    Q_INVOKABLE void sendCommand(const QString &cmd);
    Q_INVOKABLE void sendRaw(const QString &data);
    Q_INVOKABLE void scanDevices();
    Q_INVOKABLE void pairDevice(const QString &address);
    Q_INVOKABLE void connectDevice(const QString &address);
    Q_INVOKABLE void disconnectDevice();
    Q_INVOKABLE void refreshStatus();

signals:
    void connectedChanged();
    void statusTextChanged();
    void deviceNameChanged();
    void deviceAddressChanged();

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

    void serialDataReceived(const QString &data);
    void deviceFound(const QString &name, const QString &address);

private slots:
    void onReadyRead();

private:
    void handleBleJson(const QByteArray &data);
    void handleSerialRx(const QString &data);
    void parseObd2Response(const QString &response);
    void setConnected(bool c);
    void setStatusText(const QString &t);
    void setDeviceName(const QString &name);
    void setDeviceAddress(const QString &addr);

    QUdpSocket m_socket;
    QTimer *m_activityTimer = nullptr;
    QTimer *m_obd2PollTimer = nullptr;
    bool m_connected = false;
    QString m_statusText;
    QString m_deviceName;
    QString m_deviceAddress;

    // OBD2 data
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

    // Serial buffer for multi-line ELM327 responses
    QString m_serialBuffer;

    QByteArray m_buffer;
};

#endif // BLUETOOTHMANAGER_H
