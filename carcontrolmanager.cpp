#include "carcontrolmanager.h"
#include <QDebug>
#include <QTimer>
#include <QGuiApplication>
#include <QKeyEvent>
#include <QWindow>

Q_LOGGING_CATEGORY(lcCarControl, "carputer.carcontrol")

CarControlManager::CarControlManager(QObject *parent)
    : QObject(parent)
{
    m_socket = new QTcpSocket(this);
    connect(m_socket, &QTcpSocket::connected,
            this, &CarControlManager::onConnected);
    connect(m_socket, &QTcpSocket::disconnected,
            this, &CarControlManager::onDisconnected);
    connect(m_socket, &QTcpSocket::readyRead,
            this, &CarControlManager::onReadyRead);
    connect(m_socket, QOverload<QAbstractSocket::SocketError>::of(&QTcpSocket::errorOccurred),
            this, &CarControlManager::onError);
    connect(m_socket, &QTcpSocket::readyRead,
            this, &CarControlManager::onReadyRead);

    m_retryTimer = new QTimer(this);
    m_retryTimer->setInterval(1000);
    connect(m_retryTimer, &QTimer::timeout, this, &CarControlManager::tryConnect);

    tryConnect();
}

CarControlManager::~CarControlManager()
{
    if (m_socket->state() != QAbstractSocket::UnconnectedState)
        m_socket->disconnectFromHost();
}

void CarControlManager::tryConnect()
{
    if (m_socket->state() != QAbstractSocket::UnconnectedState)
        return;

    setStatus(QString("Trying %1...").arg(m_portName));
    qCInfo(lcCarControl) << "Trying to connect to" << m_portName;

    // Parse address:port
    QStringList parts = m_portName.split(":");
    if (parts.size() == 2) {
        QString host = parts.at(0);
        quint16 port = parts.at(1).toUShort();
        m_socket->connectToHost(host, port);
    } else {
        setStatus("Invalid address format: " + m_portName);
        m_retryTimer->start();
    }
}

void CarControlManager::onConnected()
{
    m_connected = true;
    m_retryTimer->stop();
    m_retryMs = 1000;
    setStatus(QString("Connected on %1").arg(m_portName));
    emit connectedChanged();
    qCInfo(lcCarControl) << "Connected to" << m_portName;
    queryStatus();
}

void CarControlManager::onDisconnected()
{
    m_connected = false;
    emit connectedChanged();
    setStatus(QString("Disconnected from %1").arg(m_portName));
    qCInfo(lcCarControl) << "Disconnected from" << m_portName;
    if (m_retryMs < 30000) m_retryMs *= 2;
    m_retryTimer->setInterval(m_retryMs);
    m_retryTimer->start();
}

void CarControlManager::onReadyRead()
{
    m_buffer.append(m_socket->readAll());

    int newlinePos;
    while ((newlinePos = m_buffer.indexOf('\n')) >= 0) {
        QString line = QString::fromLatin1(m_buffer.left(newlinePos)).trimmed();
        m_buffer.remove(0, newlinePos + 1);

        if (line.isEmpty()) continue;

        qCDebug(lcCarControl) << "Received:" << line;

        if (line.startsWith("H:")) {
            parseStatus(line);
        } else if (line == "OK") {
            qCDebug(lcCarControl) << "Command acknowledged";
        } else if (line.startsWith("ERR:")) {
            qCWarning(lcCarControl) << "Error from controller:" << line;
            emit errorOccurred(line.mid(4));
        } else if (line.startsWith("INFO:")) {
            qCInfo(lcCarControl) << "Info from controller:" << line;
        } else if (line.startsWith("WARN:")) {
            qCWarning(lcCarControl) << "Warning from controller:" << line;
        } else if (line.startsWith("J:")) {
            parseJoypad(line);
        } else if (line.startsWith("VOL:")) {
            bool ok;
            int dB = line.mid(4).toInt(&ok);
            if (ok) {
                int pct = ((dB + 31) * 100) / 51;
                if (pct < 0) pct = 0;
                if (pct > 100) pct = 100;
                emit volumeSync(pct);
            }
        }
    }
}

void CarControlManager::parseStatus(const QString &line)
{
    // Parse: "H:1 S:3 A:1 L:1 R:0"
    QStringList parts = line.split(' ');
    for (const QString &part : parts) {
        if (part.startsWith("H:")) {
            bool newValue = (part.mid(2) == "1");
            if (m_hvacEnabled != newValue) {
                m_hvacEnabled = newValue;
                emit hvacEnabledChanged();
            }
        } else if (part.startsWith("S:")) {
            int newValue = part.mid(2).toInt();
            if (m_fanSpeed != newValue) {
                m_fanSpeed = newValue;
                emit fanSpeedChanged();
            }
        } else if (part.startsWith("A:")) {
            bool newValue = (part.mid(2) == "1");
            if (m_acEnabled != newValue) {
                m_acEnabled = newValue;
                emit acEnabledChanged();
            }
        } else if (part.startsWith("L:")) {
            bool newValue = (part.mid(2) == "1");
            if (m_doorsLocked != newValue) {
                m_doorsLocked = newValue;
                emit doorsLockedChanged();
            }
        } else if (part.startsWith("R:")) {
            bool newValue = (part.mid(2) == "1");
            if (m_remoteStartActive != newValue) {
                m_remoteStartActive = newValue;
                emit remoteStartActiveChanged();
            }
        } else if (part.startsWith("F:")) {
            int newValue = part.mid(2).toInt();
            if (m_fanRelay != newValue) {
                m_fanRelay = newValue;
                emit fanRelayChanged();
            }
        } else if (part.startsWith("P:")) {
            QStringList vals = part.mid(2).split(",");
            for (int i = 0; i < 2 && i < vals.size(); i++) {
                bool newVal = (vals[i] == "1");
                if (m_extra[i] != newVal) {
                    m_extra[i] = newVal;
                    emit extraChanged();
                }
            }
        } else if (part.startsWith("K:")) {
            int newVal = part.mid(2).toInt();
            if (m_radioSource != newVal) {
                m_radioSource = newVal;
                emit radioChanged();
                qCDebug(lcCarControl) << "Radio source:" << newVal;
            }
        } else if (part.startsWith("V:")) {
            int newVal = part.mid(2).toInt();
            if (m_radioVolume != newVal) {
                m_radioVolume = newVal;
                emit radioChanged();
                qCDebug(lcCarControl) << "Radio volume:" << newVal;
            }
        } else if (part.startsWith("M:")) {
            bool newVal = (part.mid(2) == "1");
            if (m_radioMuted != newVal) {
                m_radioMuted = newVal;
                emit radioChanged();
                qCDebug(lcCarControl) << "Radio mute:" << (newVal ? "ON" : "OFF");
            }
        } else if (part.startsWith("B:")) {
            int newVal = part.mid(2).toInt();
            if (m_radioBass != newVal) {
                m_radioBass = newVal;
                emit radioChanged();
                emit radioBassChanged(newVal);
                qCDebug(lcCarControl) << "Radio bass:" << newVal;
            }
        } else if (part.startsWith("T:")) {
            int newVal = part.mid(2).toInt();
            if (m_radioTreble != newVal) {
                m_radioTreble = newVal;
                emit radioChanged();
                emit radioTrebleChanged(newVal);
                qCDebug(lcCarControl) << "Radio treble:" << newVal;
            }
        } else if (part.startsWith("D:")) {
            int newVal = part.mid(2).toInt();
            if (m_radioFader != newVal) {
                m_radioFader = newVal;
                emit radioChanged();
                qCDebug(lcCarControl) << "Radio fader:" << newVal;
            }
        } else if (part.startsWith("O:")) {
            bool newVal = (part.mid(2) == "1");
            if (m_radioLoudness != newVal) {
                m_radioLoudness = newVal;
                emit radioChanged();
                qCDebug(lcCarControl) << "Radio loudness:" << (newVal ? "ON" : "OFF");
            }
        }
    }
}

void CarControlManager::parseJoypad(const QString &line)
{
    if (line.length() < 3) return;
    QString dir = line.mid(2);
    if (dir == "U")        emit joypadUp();
    else if (dir == "D")   emit joypadDown();
    else if (dir == "L")   emit joypadLeft();
    else if (dir == "R")   emit joypadRight();
    else if (dir == "S")   emit joypadSelect();
    else if (dir == "E")   emit joypadExit();
    qCDebug(lcCarControl) << "Joypad:" << dir;
}

void CarControlManager::sendCommand(char cmd, int value)
{
    if (!m_connected || m_socket->state() != QAbstractSocket::ConnectedState) {
        qCWarning(lcCarControl) << "Cannot send command: not connected";
        emit errorOccurred("Not connected");
        return;
    }

    QByteArray data;
    data.append(cmd);
    data.append(QByteArray::number(value));
    data.append('\n');
    m_socket->write(data);
    qCDebug(lcCarControl) << "Sent command:" << cmd << "value:" << value;
}

void CarControlManager::setHvacEnabled(bool enabled)
{
    sendCommand('H', enabled ? 1 : 0);
}

void CarControlManager::setFanSpeed(int speed)
{
    if (speed < 0) speed = 0;
    if (speed > 5) speed = 5;
    sendCommand('S', static_cast<uint8_t>(speed));
}

void CarControlManager::setAcEnabled(bool enabled)
{
    sendCommand('A', enabled ? 1 : 0);
}

void CarControlManager::lockDoors()
{
    sendCommand('L', 1);
}

void CarControlManager::unlockDoors()
{
    sendCommand('L', 0);
}

void CarControlManager::windowsUp()
{
    sendCommand('W', 1);
}

void CarControlManager::windowsDown()
{
    sendCommand('W', 0);
}

void CarControlManager::startRemote()
{
    sendCommand('R', 1);
}

void CarControlManager::stopRemote()
{
    sendCommand('R', 0);
}

void CarControlManager::setFanRelay(int level)
{
    if (level < 0) level = 0;
    if (level > 2) level = 2;
    qCWarning(lcCarControl) << "setFanRelay called with level:" << level << "current m_fanRelay:" << m_fanRelay;
    if (m_fanRelay != level) {
        m_fanRelay = level;
        emit fanRelayChanged();
        qCWarning(lcCarControl) << "fanRelayChanged emitted, new value:" << m_fanRelay;
    }
    sendCommand('F', static_cast<uint8_t>(level));
}

void CarControlManager::setExtra(int index, bool on)
{
    if (index < 1 || index > 2) return;
    int encoded = index * 10 + (on ? 1 : 0);
    sendCommand('P', static_cast<uint8_t>(encoded));
}

void CarControlManager::playChime()
{
    sendCommand('C', 1);
}

void CarControlManager::setRadioSource(int source)
{
    sendCommand('K', static_cast<uint8_t>(source));
}

void CarControlManager::setRadioVolume(int volume)
{
    if (volume < -31) volume = -31;
    if (volume > 20) volume = 20;
    sendCommand('V', volume);
}

void CarControlManager::setRadioMuted(bool muted)
{
    sendCommand('M', muted ? 1 : 0);
}

void CarControlManager::setRadioBass(int value)
{
    if (value < 0) value = 0;
    if (value > 31) value = 31;
    sendCommand('B', value);
}

void CarControlManager::setRadioTreble(int value)
{
    if (value < 0) value = 0;
    if (value > 31) value = 31;
    sendCommand('T', value);
}

void CarControlManager::setRadioFader(int fader)
{
    if (fader < -15) fader = -15;
    if (fader > 15) fader = 15;
    sendCommand('D', fader);
}

void CarControlManager::setRadioLoudness(bool on)
{
    sendCommand('O', on ? 1 : 0);
}

void CarControlManager::queryStatus()
{
    if (!m_connected || m_socket->state() != QAbstractSocket::ConnectedState) {
        qCWarning(lcCarControl) << "Cannot query status: not connected";
        return;
    }
    m_socket->write("?\n");
}

void CarControlManager::setPort(const QString &port)
{
    if (port == m_portName) return;
    m_portName = port;
    emit portNameChanged();
    if (m_socket->state() != QAbstractSocket::UnconnectedState) {
        m_socket->disconnectFromHost();
        m_connected = false;
        emit connectedChanged();
    }
    tryConnect();
}

void CarControlManager::onError(QAbstractSocket::SocketError error)
{
    Q_UNUSED(error);
    qCWarning(lcCarControl) << "Socket error:" << m_socket->errorString();
    emit errorOccurred(m_socket->errorString());
    m_socket->disconnectFromHost();
    m_connected = false;
    emit connectedChanged();
    setStatus(QString("Error: %1 — retrying...").arg(m_portName));
    m_retryTimer->start();
}

void CarControlManager::setStatus(const QString &text)
{
    if (m_statusText != text) {
        m_statusText = text;
        emit statusTextChanged();
        qCDebug(lcCarControl) << text;
    }
}

void CarControlManager::injectKeyPress(int key)
{
    QWindow *window = QGuiApplication::focusWindow();
    qCDebug(lcCarControl) << "injectKeyPress key=" << key << "window=" << window;
    if (window) {
        QKeyEvent *event = new QKeyEvent(QEvent::KeyPress, static_cast<Qt::Key>(key), Qt::NoModifier);
        QCoreApplication::postEvent(window, event);
    }
}

void CarControlManager::injectKeyRelease(int key)
{
    QWindow *window = QGuiApplication::focusWindow();
    qCDebug(lcCarControl) << "injectKeyRelease key=" << key << "window=" << window;
    if (window) {
        QKeyEvent *event = new QKeyEvent(QEvent::KeyRelease, static_cast<Qt::Key>(key), Qt::NoModifier);
        QCoreApplication::postEvent(window, event);
    }
}
