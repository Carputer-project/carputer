#include "remotemanager.h"
#include <QDebug>
#include <QDir>
#include <linux/uinput.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>

Q_LOGGING_CATEGORY(lcRemote, "godseye.remote")

// ── Key map: ESP32 serial byte → Linux key code ───────────────────────────────
const RemoteManager::KeyEntry RemoteManager::KEY_MAP[] = {
    { '1',  KEY_1     },   // Video page
    { '3',  KEY_3     },   // DVR page
    { 'F',  KEY_F10   },   // Record toggle
    { 'W',  KEY_7     },   // WFB page
    { 'E',  KEY_ESC   },   // Escape (potentiometer)
    { 'P',  KEY_P     },   // Play
    { 'R',  KEY_R     },   // Restart pipeline
    { 'H',  KEY_H     },   // Toggle HUD
    { '\r', KEY_ENTER },   // Enter/Select
    { 'U',  KEY_UP    },   // Joystick up
    { 'D',  KEY_DOWN  },   // Joystick down
    { 'L',  KEY_LEFT  },   // Joystick left
    { 'G',  KEY_RIGHT },   // Joystick right
};
const int RemoteManager::KEY_MAP_SIZE = sizeof(KEY_MAP) / sizeof(KEY_MAP[0]);

// ── Constructor ───────────────────────────────────────────────────────────────
RemoteManager::RemoteManager(QObject *parent) : QObject(parent)
{
    m_serial = new QSerialPort(this);
    m_serial->setBaudRate(QSerialPort::Baud115200);
    m_serial->setDataBits(QSerialPort::Data8);
    m_serial->setParity(QSerialPort::NoParity);
    m_serial->setStopBits(QSerialPort::OneStop);
    m_serial->setFlowControl(QSerialPort::NoFlowControl);

    connect(m_serial, &QSerialPort::readyRead,
            this, &RemoteManager::onReadyRead);
    connect(m_serial, &QSerialPort::errorOccurred,
            this, &RemoteManager::onSerialError);

    // Retry timer — tries to open the port every 3 seconds until connected
    m_retryTimer = new QTimer(this);
    m_retryTimer->setInterval(3000);
    connect(m_retryTimer, &QTimer::timeout, this, &RemoteManager::tryConnect);

    setupUinput();
    tryConnect();
}

// ── Destructor ────────────────────────────────────────────────────────────────
RemoteManager::~RemoteManager()
{
    if (m_serial->isOpen())
        m_serial->close();
    closeUinput();
}

// ── uinput setup ──────────────────────────────────────────────────────────────
void RemoteManager::setupUinput()
{
    m_uinputFd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
    if (m_uinputFd < 0) {
        qCWarning(lcRemote) << "Cannot open /dev/uinput — is the uinput module loaded?"
                            << "(run: modprobe uinput)";
        setStatus("uinput unavailable — run: modprobe uinput");
        return;
    }

    // Enable key events
    ioctl(m_uinputFd, UI_SET_EVBIT, EV_KEY);
    ioctl(m_uinputFd, UI_SET_EVBIT, EV_SYN);

    // Register all keys we need
    for (int i = 0; i < KEY_MAP_SIZE; i++)
        ioctl(m_uinputFd, UI_SET_KEYBIT, KEY_MAP[i].linuxKey);

    // Create the virtual device
    struct uinput_setup usetup;
    memset(&usetup, 0, sizeof(usetup));
    usetup.id.bustype = BUS_USB;
    usetup.id.vendor  = 0x1234;
    usetup.id.product = 0x5678;
    strncpy(usetup.name, "godseye-remote", UINPUT_MAX_NAME_SIZE - 1);

    if (ioctl(m_uinputFd, UI_DEV_SETUP, &usetup) < 0 ||
        ioctl(m_uinputFd, UI_DEV_CREATE) < 0) {
        qCWarning(lcRemote) << "Failed to create uinput device";
        close(m_uinputFd);
        m_uinputFd = -1;
        setStatus("Failed to create uinput device");
        return;
    }

    qCInfo(lcRemote) << "uinput device created: godseye-remote";
}

void RemoteManager::closeUinput()
{
    if (m_uinputFd >= 0) {
        ioctl(m_uinputFd, UI_DEV_DESTROY);
        close(m_uinputFd);
        m_uinputFd = -1;
    }
}

// ── Key injection ─────────────────────────────────────────────────────────────
void RemoteManager::injectKey(int linuxKeyCode)
{
    if (m_uinputFd < 0) return;

    struct input_event ev;
    memset(&ev, 0, sizeof(ev));

    // Key down
    ev.type  = EV_KEY;
    ev.code  = linuxKeyCode;
    ev.value = 1;
    if (write(m_uinputFd, &ev, sizeof(ev)) < 0) {
        qCWarning(lcRemote) << "Failed to write key down event";
        return;
    }

    // Sync
    ev.type  = EV_SYN;
    ev.code  = SYN_REPORT;
    ev.value = 0;
    if (write(m_uinputFd, &ev, sizeof(ev)) < 0) {
        qCWarning(lcRemote) << "Failed to write sync event";
        return;
    }

    // Key up
    ev.type  = EV_KEY;
    ev.code  = linuxKeyCode;
    ev.value = 0;
    if (write(m_uinputFd, &ev, sizeof(ev)) < 0) {
        qCWarning(lcRemote) << "Failed to write key up event";
        return;
    }

    // Sync
    ev.type  = EV_SYN;
    ev.code  = SYN_REPORT;
    ev.value = 0;
    if (write(m_uinputFd, &ev, sizeof(ev)) < 0) {
        qCWarning(lcRemote) << "Failed to write final sync event";
        return;
    }

    qCDebug(lcRemote) << "Key injected:" << linuxKeyCode;
}

// ── Serial connection ─────────────────────────────────────────────────────────
void RemoteManager::tryConnect()
{
    if (m_serial->isOpen()) return;

    // Auto-detect port if not set — look for ttyUSB* devices
    if (m_portName.isEmpty() || !QDir("/dev").exists(m_portName.section('/', -1))) {
        QDir dev("/dev");
        const QStringList candidates = dev.entryList(
            QStringList() << "ttyUSB*" << "ttyACM*", QDir::System);
        if (!candidates.isEmpty())
            m_portName = "/dev/" + candidates.first();
        else {
            setStatus("Waiting for USB serial device...");
            m_retryTimer->start();
            return;
        }
    }

    m_serial->setPortName(m_portName);
    if (m_serial->open(QIODevice::ReadOnly)) {
        m_connected = true;
        m_retryTimer->stop();
        setStatus(QString("Connected on %1").arg(m_portName));
        emit connectedChanged();
        qCInfo(lcRemote) << "Remote connected on" << m_portName;
    } else {
        setStatus(QString("Waiting for remote on %1...").arg(m_portName));
        m_retryTimer->start();
    }
}

void RemoteManager::onSerialError(QSerialPort::SerialPortError error)
{
    if (error == QSerialPort::NoError) return;
    qCWarning(lcRemote) << "Serial error:" << m_serial->errorString();
    m_serial->close();
    m_connected = false;
    emit connectedChanged();
    setStatus(QString("Disconnected — retrying %1...").arg(m_portName));
    m_retryTimer->start();
}

// ── Data received ─────────────────────────────────────────────────────────────
void RemoteManager::onReadyRead()
{
    const QByteArray data = m_serial->readAll();
    for (char byte : data) {
        bool found = false;
        for (int i = 0; i < KEY_MAP_SIZE; i++) {
            if (KEY_MAP[i].code == byte) {
                injectKey(KEY_MAP[i].linuxKey);
                found = true;
                break;
            }
        }
        if (!found)
            qCDebug(lcRemote) << "Unknown byte from remote: 0x"
                              << QString::number((uint8_t)byte, 16);
    }
}

// ── Public slots ──────────────────────────────────────────────────────────────
void RemoteManager::setPort(const QString &port)
{
    if (port == m_portName) return;
    m_portName = port;
    emit portNameChanged();
    if (m_serial->isOpen()) {
        m_serial->close();
        m_connected = false;
        emit connectedChanged();
    }
    tryConnect();
}

void RemoteManager::reconnect()
{
    if (m_serial->isOpen()) {
        m_serial->close();
        m_connected = false;
        emit connectedChanged();
    }
    tryConnect();
}

// ── Helpers ───────────────────────────────────────────────────────────────────
void RemoteManager::setStatus(const QString &text)
{
    if (m_statusText == text) return;
    m_statusText = text;
    emit statusTextChanged();
    qCDebug(lcRemote) << text;
}
