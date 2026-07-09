#ifndef CARCONTROLMANAGER_H
#define CARCONTROLMANAGER_H

#include <QObject>
#include <QTcpSocket>
#include <QTimer>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(lcCarControl)

class CarControlManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool    connected    READ connected    NOTIFY connectedChanged)
    Q_PROPERTY(QString portName     READ portName     NOTIFY portNameChanged)
    Q_PROPERTY(QString statusText   READ statusText   NOTIFY statusTextChanged)

    // HVAC properties
    Q_PROPERTY(bool hvacEnabled     READ hvacEnabled  NOTIFY hvacEnabledChanged)
    Q_PROPERTY(int  fanSpeed        READ fanSpeed     NOTIFY fanSpeedChanged)
    Q_PROPERTY(bool acEnabled       READ acEnabled    NOTIFY acEnabledChanged)

    // Vehicle properties
    Q_PROPERTY(bool doorsLocked     READ doorsLocked  NOTIFY doorsLockedChanged)
    Q_PROPERTY(bool remoteStartActive READ remoteStartActive NOTIFY remoteStartActiveChanged)
    Q_PROPERTY(int  fanRelay        READ fanRelay        NOTIFY fanRelayChanged)

    // Extra relay states (ULN2003A #1 — GPIO32, GPIO33)
    Q_PROPERTY(bool extra1 READ extra1 NOTIFY extraChanged)
    Q_PROPERTY(bool extra2 READ extra2 NOTIFY extraChanged)

    // Kenwood radio state (via body controller I2C)
    Q_PROPERTY(int  radioSource  READ radioSource  NOTIFY radioChanged)
    Q_PROPERTY(int  radioVolume  READ radioVolume  NOTIFY radioChanged)
    Q_PROPERTY(bool radioMuted   READ radioMuted   NOTIFY radioChanged)
    Q_PROPERTY(int  radioBass    READ radioBass    NOTIFY radioChanged)
    Q_PROPERTY(int  radioTreble  READ radioTreble  NOTIFY radioChanged)

public:
    explicit CarControlManager(QObject *parent = nullptr);
    ~CarControlManager() override;

    // Status
    bool    connected()  const { return m_connected; }
    QString portName()   const { return m_portName; }
    QString statusText() const { return m_statusText; }

    // HVAC
    bool hvacEnabled() const { return m_hvacEnabled; }
    int  fanSpeed()    const { return m_fanSpeed; }
    bool acEnabled()   const { return m_acEnabled; }

    // Vehicle
    bool doorsLocked()       const { return m_doorsLocked; }
    bool remoteStartActive() const { return m_remoteStartActive; }
    int  fanRelay()        const { return m_fanRelay; }

    // Extra relays
    bool extra1() const { return m_extra[0]; }
    bool extra2() const { return m_extra[1]; }

    // Radio
    int  radioSource() const { return m_radioSource; }
    int  radioVolume() const { return m_radioVolume; }
    bool radioMuted()  const { return m_radioMuted; }
    int  radioBass()   const { return m_radioBass; }
    int  radioTreble() const { return m_radioTreble; }

    // Commands
    Q_INVOKABLE void setPort(const QString &port);

    Q_INVOKABLE void setHvacEnabled(bool enabled);
    Q_INVOKABLE void setFanSpeed(int speed);
    Q_INVOKABLE void setAcEnabled(bool enabled);

    Q_INVOKABLE void lockDoors();
    Q_INVOKABLE void unlockDoors();
    Q_INVOKABLE void windowsUp();
    Q_INVOKABLE void windowsDown();
    Q_INVOKABLE void startRemote();
    Q_INVOKABLE void stopRemote();
    Q_INVOKABLE void setFanRelay(int level);   // 0=off, 1=low, 2=high

    // Status query
    Q_INVOKABLE void queryStatus();

    // Extra relay control
    Q_INVOKABLE void setExtra(int index, bool on);

    // Radio control (TEA6320T via ESP32 I2C)
    Q_INVOKABLE void setRadioSource(int source);  // 0=CD, 1=Tuner, 2=AUX_C, 3=AUX_D
    Q_INVOKABLE void setRadioVolume(int volume);  // -31..20 dB
    Q_INVOKABLE void setRadioMuted(bool muted);
    Q_INVOKABLE void setRadioBass(int value);     // 0-31 (16=flat)
    Q_INVOKABLE void setRadioTreble(int value);   // 0-31 (16=flat)

    // Door chime
    Q_INVOKABLE void playChime();

    // Key injection (joypad → keyboard events)
    Q_INVOKABLE void injectKeyPress(int key);
    Q_INVOKABLE void injectKeyRelease(int key);

signals:
    void connectedChanged();
    void portNameChanged();
    void statusTextChanged();

    void hvacEnabledChanged();
    void fanSpeedChanged();
    void acEnabledChanged();

    void doorsLockedChanged();
    void remoteStartActiveChanged();
    void fanRelayChanged();

    void extraChanged();
    void radioChanged();

    void errorOccurred(const QString &error);

    // Joypad navigation signals
    void joypadUp();
    void joypadDown();
    void joypadLeft();
    void joypadRight();
    void joypadSelect();
    void joypadExit();

    // Volume sync from knob → media
    void volumeSync(int percent);

    // Radio EQ → media EQ
    void radioBassChanged(int value);
    void radioTrebleChanged(int value);

private slots:
    void onConnected();
    void onDisconnected();
    void onReadyRead();
    void onError(QAbstractSocket::SocketError error);
    void tryConnect();

private:
    void sendCommand(char cmd, int value);
    void parseStatus(const QString &line);
    void parseJoypad(const QString &line);
    void setStatus(const QString &text);

    QTcpSocket *m_socket    = nullptr;
    QTimer      *m_retryTimer  = nullptr;
    QByteArray   m_buffer;
    QString     m_portName  = "192.168.4.1:5000"; // TCP address
    QString     m_statusText  = QStringLiteral("Not connected");

    bool m_connected   = false;

    // HVAC state
    bool m_hvacEnabled = false;
    int  m_fanSpeed    = 0;
    bool m_acEnabled   = false;

    // Vehicle state
    bool m_doorsLocked       = false;
    bool m_remoteStartActive = false;
    int  m_fanRelay          = 0;

    // Extra relay states (GPIO32, GPIO33)
    bool m_extra[2]          = {false, false};

    // Kenwood radio state
    int  m_radioSource = 0;
    int  m_radioVolume = 0;
    bool m_radioMuted  = true;
    int  m_radioBass   = 16;
    int  m_radioTreble = 16;
};

#endif // CARCONTROLMANAGER_H
