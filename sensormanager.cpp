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

static int clampVal(int value, int min, int max)
{
    return value < min ? min : (value > max ? max : value);
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

    if (d.contains("speed"))        { int v = clampVal(d.value("speed").toInt(), 0, 200); if (m_speed != v) { m_speed = v; emit speedChanged(); } }
    if (d.contains("rpm"))          { int v = clampVal(d.value("rpm").toInt(), 0, 10000); if (m_rpm != v) { m_rpm = v; emit rpmChanged(); } }
    if (d.contains("throttle"))     { int v = clampVal(d.value("throttle").toInt(), 0, 100); if (m_throttle != v) { m_throttle = v; emit throttleChanged(); } }
    if (d.contains("map"))          { int v = clampVal(d.value("map").toInt(), 0, 255); if (m_map != v) { m_map = v; emit mapChanged(); } }
    if (d.contains("coolant"))      { int v = clampVal(d.value("coolant").toInt(), -40, 300); if (m_coolantTemp != v) { m_coolantTemp = v; emit coolantTempChanged(); } }
    if (d.contains("oil"))          { int v = clampVal(d.value("oil").toInt(), -40, 300); if (m_oilTemp != v) { m_oilTemp = v; emit oilTempChanged(); } }
    if (d.contains("ambient"))      { int v = clampVal(d.value("ambient").toInt(), -40, 200); if (m_ambientTemp != v) { m_ambientTemp = v; emit ambientTempChanged(); } }
    if (d.contains("intake"))       { int v = clampVal(d.value("intake").toInt(), -40, 300); if (m_intakeTemp != v) { m_intakeTemp = v; emit intakeTempChanged(); } }

    // Debug output to verify parsing
    qDebug() << "SensorManager: Parsed - speed=" << m_speed << "rpm=" << m_rpm << "coolant=" << m_coolantTemp << "throttle=" << m_throttle << "map=" << m_map;

    if (d.contains("driverDoor"))    { m_driverDoor = d.value("driverDoor").toBool(); emit driverDoorChanged(); }
    if (d.contains("passengerDoor")) { m_passengerDoor = d.value("passengerDoor").toBool(); emit passengerDoorChanged(); }
    if (d.contains("rearLeftDoor"))  { m_rearLeftDoor = d.value("rearLeftDoor").toBool(); emit rearLeftDoorChanged(); }
    if (d.contains("rearRightDoor")) { m_rearRightDoor = d.value("rearRightDoor").toBool(); emit rearRightDoorChanged(); }
    if (d.contains("trunk"))         { m_trunk = d.value("trunk").toBool(); emit trunkChanged(); }
    if (d.contains("hood"))          { m_hood = d.value("hood").toBool(); emit hoodChanged(); }

    if (d.contains("fuel"))        { int v = clampVal(d.value("fuel").toInt(), 0, 100); if (m_fuelLevel != v) { m_fuelLevel = v; emit fuelLevelChanged(); } }
    if (d.contains("oilPressure")) { int v = clampVal(d.value("oilPressure").toInt(), 0, 100); if (m_oilPressure != v) { m_oilPressure = v; emit oilPressureChanged(); } }
    if (d.contains("brakeFluid"))  { int v = clampVal(d.value("brakeFluid").toInt(), 0, 100); if (m_brakeFluid != v) { m_brakeFluid = v; emit brakeFluidChanged(); } }
    if (d.contains("battery"))    { int v = clampVal(d.value("battery").toInt(), 0, 100); if (m_battery != v) { m_battery = v; emit batteryChanged(); } }

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
