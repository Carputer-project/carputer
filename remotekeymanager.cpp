#include "remotekeymanager.h"
#include <QDebug>
#include <QRandomGenerator>

RemoteKeyManager::RemoteKeyManager(QObject *parent)
    : QObject(parent)
    , m_connected(false)
    , m_locked(true)
    , m_busy(false)
    , m_nonce(0)
{
    connect(&m_socket, &QTcpSocket::connected, this, &RemoteKeyManager::onTcpConnected);
    connect(&m_socket, &QTcpSocket::disconnected, this, &RemoteKeyManager::onTcpDisconnected);
    connect(&m_socket, &QTcpSocket::readyRead, this, &RemoteKeyManager::onTcpReadyRead);
    connect(&m_socket, &QTcpSocket::errorOccurred, this, &RemoteKeyManager::onTcpError);

    // Auto-reconnect
    m_reconnectTimer.setInterval(5000);
    connect(&m_reconnectTimer, &QTimer::timeout, this, [this]() {
        if (m_socket.state() == QAbstractSocket::UnconnectedState) {
            m_socket.connectToHost(QHostAddress("192.168.4.1"), 5000);
        }
    });
    m_reconnectTimer.start();
}

void RemoteKeyManager::lockDoors()
{
    sendAuthenticatedCommand("lock");
}

void RemoteKeyManager::unlockDoors()
{
    sendAuthenticatedCommand("unlock");
}

void RemoteKeyManager::armAlarm()
{
    sendAuthenticatedCommand("arm");
}

void RemoteKeyManager::disarmAlarm()
{
    sendAuthenticatedCommand("disarm");
}

void RemoteKeyManager::remoteStart()
{
    sendAuthenticatedCommand("remote_start");
}

void RemoteKeyManager::remoteStop()
{
    sendAuthenticatedCommand("remote_stop");
}

QString RemoteKeyManager::computeHmac(const QString &data)
{
    if (m_secret.isEmpty()) return {};
    return QCryptographicHash::hash(
        (m_vin + data + m_secret).toUtf8(),
        QCryptographicHash::Sha256
    ).toHex().left(32);
}

void RemoteKeyManager::sendAuthenticatedCommand(const QString &cmd)
{
    if (!m_connected) {
        setStatus("Not connected");
        return;
    }
    if (m_secret.isEmpty()) {
        setStatus("No secret — bind modules first");
        return;
    }

    m_nonce++;
    QString nonceStr = QString::number(m_nonce);
    QString hmac = computeHmac(cmd + nonceStr);

    QJsonObject obj;
    obj["cmd"] = cmd;
    obj["vin"] = m_vin;
    obj["nonce"] = (double)m_nonce;
    obj["hmac"] = hmac;

    QJsonDocument doc(obj);
    QByteArray data = doc.toJson(QJsonDocument::Compact) + "\n";
    m_socket.write(data);
    m_socket.flush();

    setBusy(true);
    setStatus("Sending: " + cmd);
    qDebug() << "[RemoteKey] Sent:" << cmd << "nonce:" << m_nonce;
}

void RemoteKeyManager::onTcpConnected()
{
    m_connected = true;
    emit connectedChanged();
    setStatus("Connected to body controller");
}

void RemoteKeyManager::onTcpDisconnected()
{
    m_connected = false;
    emit connectedChanged();
}

void RemoteKeyManager::onTcpReadyRead()
{
    QByteArray data = m_socket.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isObject()) return;

    QJsonObject obj = doc.object();
    QString event = obj["event"].toString();

    if (event == "cmd_ack") {
        QString cmd = obj["cmd"].toString();
        bool locked = obj["locked"].toBool();
        m_locked = locked;
        emit lockedChanged();
        setBusy(false);
        setStatus("OK: " + cmd);
        emit commandAck(cmd);
    } else if (event == "cmd_nack") {
        QString cmd = obj["cmd"].toString();
        QString reason = obj["reason"].toString();
        setBusy(false);
        setStatus("Failed: " + reason);
        emit commandNack(cmd, reason);
    }
}

void RemoteKeyManager::onTcpError(QAbstractSocket::SocketError error)
{
    Q_UNUSED(error)
    if (m_connected) {
        m_connected = false;
        emit connectedChanged();
    }
}

void RemoteKeyManager::setStatus(const QString &msg)
{
    m_statusText = msg;
    emit statusTextChanged();
}

void RemoteKeyManager::setBusy(bool busy)
{
    if (m_busy == busy) return;
    m_busy = busy;
    emit busyChanged();
}
