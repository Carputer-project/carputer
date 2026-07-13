#include "bluetoothmanager.h"
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>

BluetoothManager::BluetoothManager(QObject *parent)
    : QObject(parent)
{
    // Bind UDP socket to receive serial bridge data from ESP32 (port 5004)
    bool bound = m_socket.bind(QHostAddress::Any, 5004, QUdpSocket::ShareAddress);
    if (bound) {
        setStatusText("Bluetooth bridge ready (listening on UDP 5004)");
        qDebug() << "BluetoothManager: listening on UDP port 5004";
    } else {
        setStatusText("Failed to bind UDP socket on port 5004");
        qWarning() << "BluetoothManager: failed to bind UDP socket";
    }
    connect(&m_socket, &QUdpSocket::readyRead, this, &BluetoothManager::onReadyRead);

    // Activity timer — if no data for 10 seconds, mark as disconnected
    m_activityTimer = new QTimer(this);
    m_activityTimer->setInterval(10000);
    connect(m_activityTimer, &QTimer::timeout, this, [this]() {
        setConnected(false);
        setStatusText("No Bluetooth data received (timeout)");
    });
    m_activityTimer->start();

    // OBD2 poll timer — queries ELM327 every 1 second
    m_obd2PollTimer = new QTimer(this);
    m_obd2PollTimer->setInterval(1000);
    connect(m_obd2PollTimer, &QTimer::timeout, this, [this]() {
        if (!m_connected) return;
        // Query standard OBD2 PIDs
        sendCommand("0100");   // PIDs supported (bitmask)
        sendCommand("0104");   // Engine load %
        sendCommand("0105");   // Coolant temp
        sendCommand("0106");   // Short term fuel trim
        sendCommand("0107");   // Long term fuel trim
        sendCommand("010C");   // RPM
        sendCommand("010D");   // Vehicle speed
        sendCommand("010E");   // Timing advance
        sendCommand("010F");   // Intake air temp
        sendCommand("0121");   // Distance since DTC cleared
    });
}

BluetoothManager::~BluetoothManager()
{
    m_obd2PollTimer->stop();
    m_socket.close();
}

// ── Send AT command to HC-05 via ESP32 serial bridge ─────────────────
void BluetoothManager::sendCommand(const QString &cmd)
{
    if (cmd.isEmpty()) return;

    // Send to ESP32 as serial_tx command
    QJsonObject doc;
    doc["cmd"] = "serial_tx";
    doc["data"] = cmd + "\r\n";

    QByteArray packet = QJsonDocument(doc).toJson(QJsonDocument::Compact);

    // Send to ESP32 sensor module (192.168.4.20:5002)
    m_socket.writeDatagram(packet, QHostAddress("192.168.4.20"), 5002);
    qDebug() << "BluetoothManager: TX:" << cmd;
}

void BluetoothManager::sendRaw(const QString &data)
{
    if (data.isEmpty()) return;

    QJsonObject doc;
    doc["cmd"] = "serial_tx";
    doc["data"] = data;

    QByteArray packet = QJsonDocument(doc).toJson(QJsonDocument::Compact);
    m_socket.writeDatagram(packet, QHostAddress("192.168.4.20"), 5002);
    qDebug() << "BluetoothManager: RAW TX:" << data;
}

// ── High-level BT operations (send AT commands to HC-05) ─────────────
void BluetoothManager::scanDevices()
{
    setStatusText("Scanning for Bluetooth devices...");
    sendCommand("AT+INQ");  // HC-05 inquiry/scan
}

void BluetoothManager::pairDevice(const QString &address)
{
    // HC-05 pairing: AT+PAIR=<address>,<role>
    // Address format: XXXX,XX,XXXXXX (HC-05 format, not XX:XX:XX:XX:XX:XX)
    sendCommand("AT+PAIR=" + address + ",5");  // role 5 = slave
    setStatusText("Pairing with " + address + "...");
}

void BluetoothManager::connectDevice(const QString &address)
{
    // HC-05 connect: AT+LINK=<address>
    sendCommand("AT+LINK=" + address);
    setStatusText("Connecting to " + address + "...");
}

void BluetoothManager::disconnectDevice()
{
    sendCommand("AT+DISC");
    setConnected(false);
    setDeviceName("");
    setDeviceAddress("");
    setStatusText("Disconnected");
    m_obd2PollTimer->stop();
}

void BluetoothManager::refreshStatus()
{
    sendCommand("AT+STATE");  // Query current HC-05 state
    sendCommand("AT+NAME?");  // Query HC-05 name
    sendCommand("AT+ADDR?");  // Query HC-05 address
}

// ── UDP receive handler ──────────────────────────────────────────────
void BluetoothManager::onReadyRead()
{
    while (m_socket.hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(m_socket.pendingDatagramSize());
        QHostAddress sender;
        quint16 senderPort;
        m_socket.readDatagram(datagram.data(), datagram.size(), &sender, &senderPort);

        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson(datagram, &parseError);
        if (parseError.error != QJsonParseError::NoError || !doc.isObject())
            continue;

        QJsonObject obj = doc.object();
        QString event = obj.value("event").toString();
        if (event != "ble") continue;

        QString type = obj.value("type").toString();

        if (type == "serial_rx") {
            handleSerialRx(obj.value("data").toString());
        } else if (type == "status") {
            bool wasConnected = m_connected;
            bool hwConnected = obj.value("connected").toBool();
            setConnected(hwConnected);

            if (hwConnected && !wasConnected) {
                setStatusText("Bluetooth device connected — initializing ELM327...");
                // Auto-initialize ELM327 when HC-05 connects
                QTimer::singleShot(500, this, [this]() {
                    sendCommand("ATZ");      // Reset ELM327
                    sendCommand("ATE0");     // Echo off
                    sendCommand("ATSP0");    // Auto-detect protocol
                    m_obd2PollTimer->start(); // Start OBD2 polling
                });
            } else if (!hwConnected && wasConnected) {
                setStatusText("Bluetooth device disconnected");
                m_obd2PollTimer->stop();
                setDeviceName("");
                setDeviceAddress("");
            }
        }

        m_activityTimer->start();
    }
}

// ── Handle serial data from HC-05/ELM327 ─────────────────────────────
void BluetoothManager::handleSerialRx(const QString &data)
{
    m_serialBuffer += data;

    // Process line by line (ELM327 sends \r\n terminated responses)
    while (m_serialBuffer.contains('\n')) {
        int idx = m_serialBuffer.indexOf('\n');
        QString line = m_serialBuffer.left(idx).trimmed();
        m_serialBuffer.remove(0, idx + 1);

        if (line.isEmpty()) continue;

        qDebug() << "BluetoothManager: RX:" << line;

        // Filter out AT echo and prompts
        if (line.startsWith("AT") || line == "OK" || line == ">" ||
            line.startsWith("ELM") || line == "SENDING..." || line == "BUS INIT...") {
            continue;
        }

        // Parse OBD2 response (hex format: "41 XX YY [ZZ]")
        if (line.length() >= 2) {
            bool ok;
            int firstByte = line.left(2).toInt(&ok, 16);
            if (ok && firstByte == 0x41) {
                parseObd2Response(line);
            }
        }

        // Emit raw serial data for QML to display
        emit serialDataReceived(line);
    }
}

// ── Parse OBD2 response ──────────────────────────────────────────────
void BluetoothManager::parseObd2Response(const QString &response)
{
    // Response format: "41 XX YY [ZZ]"
    // XX = PID, YY/ZZ = data bytes
    QStringList parts = response.split(' ', Qt::SkipEmptyParts);
    if (parts.size() < 3) return;

    bool ok;
    int pid = parts[1].toInt(&ok, 16);
    if (!ok) return;

    int dataA = parts[2].toInt(&ok, 16);
    if (!ok) return;

    int dataB = (parts.size() > 3) ? parts[3].toInt(&ok, 16) : 0;

    switch (pid) {
    case 0x04: { // Calculated engine load (%)
        int v = (int)(dataA * 100.0 / 255.0);
        if (m_obd2Load != v) { m_obd2Load = v; emit obd2LoadChanged(); }
        break;
    }
    case 0x05: { // Engine coolant temperature (°C → °F)
        int v = (int)((dataA - 40) * 9.0 / 5.0 + 32);
        if (m_obd2Coolant != v) { m_obd2Coolant = v; emit obd2CoolantChanged(); }
        break;
    }
    case 0x06: { // Short term fuel trim (%)
        int v = (int)((dataA - 128) * 100.0 / 128.0);
        if (m_obd2FuelTrimShort != v) { m_obd2FuelTrimShort = v; emit obd2FuelTrimShortChanged(); }
        break;
    }
    case 0x07: { // Long term fuel trim (%)
        int v = (int)((dataA - 128) * 100.0 / 128.0);
        if (m_obd2FuelTrimLong != v) { m_obd2FuelTrimLong = v; emit obd2FuelTrimLongChanged(); }
        break;
    }
    case 0x0C: { // RPM (A*256+B)/4
        int v = (int)((dataA * 256 + dataB) / 4.0);
        if (m_obd2Rpm != v) { m_obd2Rpm = v; emit obd2RpmChanged(); }
        break;
    }
    case 0x0D: { // Vehicle speed (km/h)
        if (m_obd2Speed != dataA) { m_obd2Speed = dataA; emit obd2SpeedChanged(); }
        break;
    }
    case 0x0E: { // Timing advance (degrees before TDC)
        int v = (int)(dataA / 2.0 - 64.0);
        if (m_obd2Timing != v) { m_obd2Timing = v; emit obd2TimingChanged(); }
        break;
    }
    case 0x0F: { // Intake air temperature (°C → °F)
        int v = (int)((dataA - 40) * 9.0 / 5.0 + 32);
        if (m_obd2Intake != v) { m_obd2Intake = v; emit obd2IntakeChanged(); }
        break;
    }
    case 0x21: { // Distance since DTC cleared (km)
        int v = dataA * 256 + dataB;
        if (m_obd2DistanceDtc != v) { m_obd2DistanceDtc = v; emit obd2DistanceDtcChanged(); }
        break;
    }
    default:
        break;
    }
}

// ── Property setters ─────────────────────────────────────────────────
void BluetoothManager::setConnected(bool c)
{
    if (m_connected != c) {
        m_connected = c;
        emit connectedChanged();
    }
}

void BluetoothManager::setStatusText(const QString &t)
{
    if (m_statusText != t) {
        m_statusText = t;
        emit statusTextChanged();
    }
}

void BluetoothManager::setDeviceName(const QString &name)
{
    if (m_deviceName != name) {
        m_deviceName = name;
        emit deviceNameChanged();
    }
}

void BluetoothManager::setDeviceAddress(const QString &addr)
{
    if (m_deviceAddress != addr) {
        m_deviceAddress = addr;
        emit deviceAddressChanged();
    }
}
