#pragma once
#include <QObject>
#include <QSerialPort>
#include <QTimer>
#include <QLoggingCategory>
#include <linux/uinput.h>
#include <fcntl.h>
#include <unistd.h>

Q_DECLARE_LOGGING_CATEGORY(lcRemote)

// ── RemoteManager ─────────────────────────────────────────────────────────────
// Reads single-byte key codes from an ESP32 remote over serial (USB or HC-05
// adapter on /dev/ttyUSB0) and injects them as real Linux keypresses via uinput.
//
// Key code map (matches ESP32 firmware):
//   '1'  → KEY_1       Video page
//   '3'  → KEY_3       DVR page
//   'F'  → KEY_F10     Record toggle
//   'P'  → KEY_P       Play
//   'R'  → KEY_R       Restart pipeline
//   'H'  → KEY_H       Toggle HUD
//   '\r' → KEY_ENTER   Select / confirm
//   'U'  → KEY_UP      Joystick up
//   'D'  → KEY_DOWN    Joystick down
//   'L'  → KEY_LEFT    Joystick left
//   'G'  → KEY_RIGHT   Joystick right

class RemoteManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool    connected    READ connected    NOTIFY connectedChanged)
    Q_PROPERTY(QString portName     READ portName     NOTIFY portNameChanged)
    Q_PROPERTY(QString statusText   READ statusText   NOTIFY statusTextChanged)

public:
    explicit RemoteManager(QObject *parent = nullptr);
    ~RemoteManager() override;

    bool    connected()  const { return m_connected; }
    QString portName()   const { return m_portName; }
    QString statusText() const { return m_statusText; }

    Q_INVOKABLE void setPort(const QString &port);
    Q_INVOKABLE void reconnect();

signals:
    void connectedChanged();
    void portNameChanged();
    void statusTextChanged();

private slots:
    void onReadyRead();
    void onSerialError(QSerialPort::SerialPortError error);
    void tryConnect();

private:
    void     injectKey(int linuxKeyCode);
    void     setupUinput();
    void     closeUinput();
    void     setStatus(const QString &text);

    QSerialPort *m_serial      = nullptr;
    QTimer      *m_retryTimer  = nullptr;
    int          m_uinputFd    = -1;
    bool         m_connected   = false;
    QString      m_portName    = QStringLiteral("/dev/ttyUSB0");
    QString      m_statusText  = QStringLiteral("Not connected");

    // Map from ESP32 byte code → Linux key code
    struct KeyEntry { char code; int linuxKey; };
    static const KeyEntry KEY_MAP[];
    static const int      KEY_MAP_SIZE;
};
