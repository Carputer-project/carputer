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

    void errorOccurred(const QString &error);

private slots:
    void onConnected();
    void onDisconnected();
    void onReadyRead();
    void onError(QAbstractSocket::SocketError error);
    void tryConnect();

private:
    void sendCommand(char cmd, uint8_t value);
    void parseStatus(const QString &line);
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
};

#endif // CARCONTROLMANAGER_H
