#include "carcontrolmanager.h"
#include <QDebug>
#include <QTimer>

Q_LOGGING_CATEGORY(lcCarControl, "carputer.carcontrol")

CarControlManager::CarControlManager(QObject *parent)
    : QObject(parent)
{
    m_socket = new QTcpSocket(this);
    connect(m_socket, &QTcpSocket::connected,
            this, &CarControlManager::onConnected);
    connect(m_socket, &QTcpSocket::disconnected,
            this, &CarControlManager::onDisconnected);
    connect(m_socket, QOverload<QAbstractSocket::SocketError>::of(&QTcpSocket::errorOccurred),
            this, &CarControlManager::onError);

    m_retryTimer = new QTimer(this);
    m_retryTimer->setInterval(3000);
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
        }
    }
}

void CarControlManager::sendCommand(char cmd, uint8_t value)
{
    if (!m_connected || m_socket->state() != QAbstractSocket::ConnectedState) {
        qCWarning(lcCarControl) << "Cannot send command: not connected";
        emit errorOccurred("Not connected");
        return;
    }

    QByteArray data;
    data.append(cmd);
    data.append(static_cast<char>(value));
    data.append('\n');  // Newline for text protocol
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
