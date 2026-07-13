#include "v2vmanager.h"
#include <QDebug>
#include <QDateTime>

V2vManager::V2vManager(QObject *parent)
    : QObject(parent)
    , m_meshActive(false)
    , m_lastPacketTime(0)
{
    connect(&m_socket, &QUdpSocket::readyRead, this, &V2vManager::onUdpReadyRead);

    // Bind to mesh port for receiving broadcasts
    m_socket.bind(QHostAddress::Any, 5010);
    qDebug() << "[V2vManager] Listening on UDP 5010";

    // Expire timer — check every second
    m_expireTimer.setInterval(1000);
    connect(&m_expireTimer, &QTimer::timeout, this, &V2vManager::onExpireTimer);
    m_expireTimer.start();
}

QString V2vManager::carVin(int index) const
{
    if (index < 0 || index >= m_cars.size()) return {};
    return m_cars[index].vin;
}

int V2vManager::carSpeed(int index) const
{
    if (index < 0 || index >= m_cars.size()) return 0;
    return m_cars[index].speed;
}

int V2vManager::carRpm(int index) const
{
    if (index < 0 || index >= m_cars.size()) return 0;
    return m_cars[index].rpm;
}

int V2vManager::carGear(int index) const
{
    if (index < 0 || index >= m_cars.size()) return 0;
    return m_cars[index].gear;
}

int V2vManager::carHop(int index) const
{
    if (index < 0 || index >= m_cars.size()) return 0;
    return m_cars[index].hop;
}

int V2vManager::carCoolant(int index) const
{
    if (index < 0 || index >= m_cars.size()) return 0;
    return m_cars[index].coolant;
}

int V2vManager::carThrottle(int index) const
{
    if (index < 0 || index >= m_cars.size()) return 0;
    return m_cars[index].throttle;
}

QString V2vManager::carFreshness(int index) const
{
    if (index < 0 || index >= m_cars.size()) return {};
    unsigned long age = QDateTime::currentMSecsSinceEpoch() - m_cars[index].lastSeen;
    if (age < 2000) return "fresh";
    if (age < 5000) return "stale";
    return "old";
}

void V2vManager::onUdpReadyRead()
{
    while (m_socket.hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(m_socket.pendingDatagramSize());
        m_socket.readDatagram(datagram.data(), datagram.size());
        processPacket(datagram);
    }
}

void V2vManager::processPacket(const QByteArray &data)
{
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
    if (parseError.error != QJsonParseError::NoError || !doc.isObject()) return;

    QJsonObject obj = doc.object();
    QString type = obj["type"].toString();

    if (type == "telemetry") {
        QString vin = obj["vin"].toString();
        int hop = obj["hop"].toInt();
        QJsonObject dataObj = obj["data"].toObject();

        m_lastPacketTime = QDateTime::currentMSecsSinceEpoch();
        if (!m_meshActive) {
            m_meshActive = true;
            emit meshActiveChanged();
        }

        // Find or add car
        bool found = false;
        for (auto& car : m_cars) {
            if (car.vin == vin) {
                car.hop = hop;
                car.lastSeen = QDateTime::currentMSecsSinceEpoch();
                car.speed = dataObj["speed"].toInt();
                car.rpm = dataObj["rpm"].toInt();
                car.gear = dataObj["gear"].toInt();
                car.coolant = dataObj["coolant"].toInt();
                car.throttle = dataObj["throttle"].toInt();
                found = true;
                break;
            }
        }
        if (!found) {
            MeshCarInfo car;
            car.vin = vin;
            car.moduleId = obj["moduleId"].toString();
            car.hop = hop;
            car.lastSeen = QDateTime::currentMSecsSinceEpoch();
            car.speed = dataObj["speed"].toInt();
            car.rpm = dataObj["rpm"].toInt();
            car.gear = dataObj["gear"].toInt();
            car.coolant = dataObj["coolant"].toInt();
            car.throttle = dataObj["throttle"].toInt();
            m_cars.append(car);
            emit carCountChanged();
            emit carDiscovered(vin, car.speed, car.rpm);
        }
    } else if (type == "alert") {
        QString vin = obj["vin"].toString();
        QString alertType = obj["alertType"].toString();
        emit alertReceived(vin, alertType);
    }
}

void V2vManager::onExpireTimer()
{
    if (m_cars.isEmpty()) return;

    unsigned long now = QDateTime::currentMSecsSinceEpoch();
    bool changed = false;

    for (int i = m_cars.size() - 1; i >= 0; i--) {
        if (now - m_cars[i].lastSeen > 5000) {
            emit carLost(m_cars[i].vin);
            m_cars.removeAt(i);
            changed = true;
        }
    }

    if (changed) emit carCountChanged();

    // Check if mesh went inactive
    if (m_meshActive && m_lastPacketTime > 0 && now - m_lastPacketTime > 10000) {
        m_meshActive = false;
        emit meshActiveChanged();
    }
}
