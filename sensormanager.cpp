#include "sensormanager.h"
#include <QDebug>
#include <QNetworkInterface>
#include <QTimer>

SensorManager::SensorManager(QObject *parent)
    : QObject(parent)
{
    // Bind UDP socket to receive sensor data from ESP32 (192.168.4.20:5001)
    bool bound = m_socket.bind(QHostAddress::Any, 5001, QUdpSocket::ShareAddress);
    if (bound) {
        setConnected(false);  // Not truly connected until data received
        setStatusText("Listening for sensor data on UDP 5001");
        qDebug() << "SensorManager: listening on UDP port 5001";
    } else {
        setConnected(false);
        setStatusText("Failed to bind UDP socket on port 5001");
        qWarning() << "SensorManager: failed to bind UDP socket";
    }
    connect(&m_socket, &QUdpSocket::readyRead, this, &SensorManager::onReadyRead);

    // Activity timer - if no data received for 5 seconds, mark as disconnected
    m_activityTimer = new QTimer(this);
    m_activityTimer->setInterval(5000);
    connect(m_activityTimer, &QTimer::timeout, this, [this]() {
        setConnected(false);
        setStatusText("No sensor data received (timeout)");
    });
    m_activityTimer->start();
}

SensorManager::~SensorManager()
{
    m_socket.close();
}

void SensorManager::reconnect()
{
    setConnected(false);
    setStatusText("Rebinding UDP socket...");
    m_socket.close();
    if (m_socket.bind(QHostAddress::Any, 5001, QUdpSocket::ShareAddress)) {
        setConnected(true);
        setStatusText("Listening for sensor data on UDP 5001");
    } else {
        setStatusText("Failed to rebind UDP socket");
    }
}

void SensorManager::onReadyRead()
{
    while (m_socket.hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(m_socket.pendingDatagramSize());
        QHostAddress sender;
        quint16 senderPort;
        m_socket.readDatagram(datagram.data(), datagram.size(), &sender, &senderPort);

        QString senderStr = sender.toString();
        qDebug() << "SensorManager: received" << datagram.size() << "bytes from" << senderStr << ":" << senderPort;
        parseSensorJson(datagram);

        // Mark as connected since we received data
        setConnected(true);
        setStatusText("Receiving sensor data from " + senderStr);
        m_activityTimer->start();  // Reset activity timer
    }
}

void SensorManager::parseSensorJson(const QByteArray &data)
{
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        qDebug() << "SensorManager: JSON parse error:" << parseError.errorString() << "data:" << data;
        return;
    }
    if (!doc.isObject())
        return;

    QJsonObject obj = doc.object();
    QString event = obj.value("event").toString();
    if (event != "sensors")
        return;

    QJsonObject d = obj.value("data").toObject();

    if (d.contains("speed"))        { m_speed = d.value("speed").toInt(); emit speedChanged(); }
    if (d.contains("rpm"))          { m_rpm = d.value("rpm").toInt(); emit rpmChanged(); }
    if (d.contains("coolant"))      { m_coolantTemp = d.value("coolant").toInt(); emit coolantTempChanged(); }
    if (d.contains("oil"))          { m_oilTemp = d.value("oil").toInt(); emit oilTempChanged(); }
    if (d.contains("ambient"))     { m_ambientTemp = d.value("ambient").toInt(); emit ambientTempChanged(); }
    if (d.contains("intake"))     { m_intakeTemp = d.value("intake").toInt(); emit intakeTempChanged(); }

    // Debug output to verify parsing
    qDebug() << "SensorManager: Parsed - speed=" << m_speed << "coolant=" << m_coolantTemp << "fuel=" << m_fuelLevel;

    if (d.contains("driverDoor"))    { m_driverDoor = d.value("driverDoor").toBool(); emit driverDoorChanged(); }
    if (d.contains("passengerDoor")) { m_passengerDoor = d.value("passengerDoor").toBool(); emit passengerDoorChanged(); }
    if (d.contains("rearLeftDoor"))  { m_rearLeftDoor = d.value("rearLeftDoor").toBool(); emit rearLeftDoorChanged(); }
    if (d.contains("rearRightDoor")) { m_rearRightDoor = d.value("rearRightDoor").toBool(); emit rearRightDoorChanged(); }
    if (d.contains("trunk"))         { m_trunk = d.value("trunk").toBool(); emit trunkChanged(); }
    if (d.contains("hood"))          { m_hood = d.value("hood").toBool(); emit hoodChanged(); }

    if (d.contains("fuel"))       { m_fuelLevel = d.value("fuel").toInt(); emit fuelLevelChanged(); }
    if (d.contains("oilPressure")){ m_oilPressure = d.value("oilPressure").toInt(); emit oilPressureChanged(); }
    if (d.contains("brakeFluid")){ m_brakeFluid = d.value("brakeFluid").toInt(); emit brakeFluidChanged(); }
    if (d.contains("battery"))   { m_battery = d.value("battery").toInt(); emit batteryChanged(); }

    emit sensorDataReceived(QString::fromUtf8(data));
}

void SensorManager::setConnected(bool c)
{
    if (m_connected != c) {
        m_connected = c;
        emit connectedChanged();
    }
}

void SensorManager::setStatusText(const QString &t)
{
    if (m_statusText != t) {
        m_statusText = t;
        emit statusTextChanged();
    }
}
