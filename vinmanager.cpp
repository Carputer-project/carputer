#include "vinmanager.h"
#include <QDebug>
#include <QRandomGenerator>
#include <QDateTime>
#include <QNetworkInterface>

VinManager::VinManager(QObject *parent)
    : QObject(parent)
    , m_settings("/etc/carputer/vin.conf", QSettings::IniFormat)
    , m_bodyBound(false)
    , m_sensorBound(false)
    , m_meshBound(false)
    , m_remoteBound(false)
    , m_busy(false)
{
    loadConfig();

    // Status timer — clear status after 5 seconds
    m_statusTimer.setSingleShot(true);
    m_statusTimer.setInterval(5000);
    connect(&m_statusTimer, &QTimer::timeout, this, [this]() {
        m_statusText.clear();
        emit statusTextChanged();
    });

    // Sensor module replies via UDP (port 5001 — same as sensor data)
    connect(&m_sensorSocket, &QUdpSocket::readyRead, this, &VinManager::onSensorBindReply);

    // Body controller replies via TCP
    connect(&m_bodySocket, &QTcpSocket::readyRead, this, &VinManager::onBodyBindReply);
    connect(&m_bodySocket, &QTcpSocket::connected, this, [this]() {
        qDebug() << "[VinManager] Body controller TCP connected";
    });
    connect(&m_bodySocket, &QTcpSocket::errorOccurred, this, [this](QAbstractSocket::SocketError) {
        qDebug() << "[VinManager] Body controller TCP error:" << m_bodySocket.errorString();
        if (m_pendingBindType == "body") {
            setBusy(false);
            emit bindFailed("body", m_bodySocket.errorString());
            setStatus("Body bind failed: " + m_bodySocket.errorString());
        }
    });
}

void VinManager::loadConfig()
{
    m_vin = m_settings.value("vin").toString();
    m_secret = m_settings.value("secret").toString();

    // Body Controller
    m_bodyBound = m_settings.value("body/bound", false).toBool();
    m_bodyModuleId = m_settings.value("body/moduleId").toString();
    m_bodyMac = m_settings.value("body/mac").toString();

    // Sensor Module
    m_sensorBound = m_settings.value("sensor/bound", false).toBool();
    m_sensorModuleId = m_settings.value("sensor/moduleId").toString();
    m_sensorMac = m_settings.value("sensor/mac").toString();

    // Mesh Module
    m_meshBound = m_settings.value("mesh/bound", false).toBool();
    m_meshModuleId = m_settings.value("mesh/moduleId").toString();
    m_meshMac = m_settings.value("mesh/mac").toString();

    // Remote Fob
    m_remoteBound = m_settings.value("remote/bound", false).toBool();
    m_remoteModuleId = m_settings.value("remote/moduleId").toString();
    m_remoteMac = m_settings.value("remote/mac").toString();

    if (!m_vin.isEmpty()) emit vinChanged();
    if (!m_secret.isEmpty()) emit secretChanged();
    if (m_bodyBound) { emit bodyBoundChanged(); emit bodyModuleIdChanged(); emit bodyMacChanged(); }
    if (m_sensorBound) { emit sensorBoundChanged(); emit sensorModuleIdChanged(); emit sensorMacChanged(); }
    if (m_meshBound) { emit meshBoundChanged(); emit meshModuleIdChanged(); emit meshMacChanged(); }
    if (m_remoteBound) { emit remoteBoundChanged(); emit remoteModuleIdChanged(); emit remoteMacChanged(); }
}

void VinManager::saveConfig()
{
    m_settings.setValue("vin", m_vin);
    m_settings.setValue("secret", m_secret);

    m_settings.setValue("body/bound", m_bodyBound);
    m_settings.setValue("body/moduleId", m_bodyModuleId);
    m_settings.setValue("body/mac", m_bodyMac);

    m_settings.setValue("sensor/bound", m_sensorBound);
    m_settings.setValue("sensor/moduleId", m_sensorModuleId);
    m_settings.setValue("sensor/mac", m_sensorMac);

    m_settings.setValue("mesh/bound", m_meshBound);
    m_settings.setValue("mesh/moduleId", m_meshModuleId);
    m_settings.setValue("mesh/mac", m_meshMac);

    m_settings.setValue("remote/bound", m_remoteBound);
    m_settings.setValue("remote/moduleId", m_remoteModuleId);
    m_settings.setValue("remote/mac", m_remoteMac);

    m_settings.sync();
}

void VinManager::generateSecret()
{
    QByteArray randomBytes;
    randomBytes.resize(32);
    for (int i = 0; i < 32; i++)
        randomBytes[i] = (char)QRandomGenerator::global()->bounded(256);
    m_secret = randomBytes.toHex();
    emit secretChanged();
}

QString VinManager::generateModuleId(const QString &prefix, const QString &mac, const QString &vinStr)
{
    QByteArray data = (mac + vinStr + QDateTime::currentDateTimeUtc().toString(Qt::ISODate)).toUtf8();
    QString hash = QCryptographicHash::hash(data, QCryptographicHash::Sha256).toHex().left(8);
    return prefix + "-" + hash.toUpper();
}

// --- Setters ---

void VinManager::setVin(const QString &vin)
{
    QString normalized = vin.trimmed().toUpper();
    if (normalized == m_vin)
        return;

    // Validate: exactly 17 alphanumeric characters
    if (!normalized.isEmpty() && !QRegularExpression("^[A-HJ-NPR-Z0-9]{17}$").match(normalized).hasMatch()) {
        setStatus("Invalid VIN — must be 17 alphanumeric chars (no I, O, Q)");
        return;
    }

    m_vin = normalized;
    m_settings.setValue("vin", m_vin);
    m_settings.sync();
    emit vinChanged();
    setStatus("VIN set: " + m_vin);
}

// --- Bind actions ---

void VinManager::bindBody()
{
    if (m_vin.isEmpty()) {
        setStatus("Set VIN first");
        return;
    }
    setStatus("Binding body controller...");
    setBusy(true);
    m_pendingBindType = "body";
    m_pendingBindVin = m_vin;
    sendBindToBody(m_vin);
}

void VinManager::bindSensor()
{
    if (m_vin.isEmpty()) {
        setStatus("Set VIN first");
        return;
    }
    setStatus("Binding sensor module...");
    setBusy(true);
    m_pendingBindType = "sensor";
    m_pendingBindVin = m_vin;
    sendBindToSensor(m_vin);
}

void VinManager::bindMesh()
{
    // Mesh module not yet created — store locally for now
    if (m_vin.isEmpty()) {
        setStatus("Set VIN first");
        return;
    }
    QString moduleId = generateModuleId("mesh", "00:00:00:00:00:00", m_vin);
    m_meshBound = true;
    m_meshModuleId = moduleId;
    m_meshMac = "00:00:00:00:00:00";
    saveConfig();
    emit meshBoundChanged();
    emit meshModuleIdChanged();
    emit meshMacChanged();
    setStatus("Mesh module bound: " + moduleId);
    emit bindSucceeded("mesh", moduleId);
}

void VinManager::bindRemote()
{
    // Remote fob not yet created — store locally for now
    if (m_vin.isEmpty()) {
        setStatus("Set VIN first");
        return;
    }
    QString moduleId = generateModuleId("RK", "00:00:00:00:00:00", m_vin);
    m_remoteBound = true;
    m_remoteModuleId = moduleId;
    m_remoteMac = "00:00:00:00:00:00";
    saveConfig();
    emit remoteBoundChanged();
    emit remoteModuleIdChanged();
    emit remoteMacChanged();
    setStatus("Remote fob bound: " + moduleId);
    emit bindSucceeded("remote", moduleId);
}

// --- Unbind actions ---

void VinManager::unbindBody()
{
    sendUnbindToBody();
    m_bodyBound = false;
    m_bodyModuleId.clear();
    m_bodyMac.clear();
    saveConfig();
    emit bodyBoundChanged();
    emit bodyModuleIdChanged();
    emit bodyMacChanged();
    setStatus("Body controller unbound");
    emit unbindSucceeded("body");
}

void VinManager::unbindSensor()
{
    sendUnbindToSensor();
    m_sensorBound = false;
    m_sensorModuleId.clear();
    m_sensorMac.clear();
    saveConfig();
    emit sensorBoundChanged();
    emit sensorModuleIdChanged();
    emit sensorMacChanged();
    setStatus("Sensor module unbound");
    emit unbindSucceeded("sensor");
}

void VinManager::unbindMesh()
{
    m_meshBound = false;
    m_meshModuleId.clear();
    m_meshMac.clear();
    saveConfig();
    emit meshBoundChanged();
    emit meshModuleIdChanged();
    emit meshMacChanged();
    setStatus("Mesh module unbound");
    emit unbindSucceeded("mesh");
}

void VinManager::unbindRemote()
{
    m_remoteBound = false;
    m_remoteModuleId.clear();
    m_remoteMac.clear();
    saveConfig();
    emit remoteBoundChanged();
    emit remoteModuleIdChanged();
    emit remoteMacChanged();
    setStatus("Remote fob unbound");
    emit unbindSucceeded("remote");
}

void VinManager::unbindAll()
{
    unbindBody();
    unbindSensor();
    unbindMesh();
    unbindRemote();
    m_secret.clear();
    emit secretChanged();
    setStatus("All modules unbound");
    emit allUnbound();
}

// --- Network send ---

void VinManager::sendBindToSensor(const QString &vinStr)
{
    QJsonObject cmd;
    cmd["cmd"] = "bind";
    cmd["vin"] = vinStr;
    QJsonDocument doc(cmd);
    QByteArray data = doc.toJson(QJsonDocument::Compact);

    // Send to sensor module (UDP port 5002 — sensor command port)
    m_sensorSocket.writeDatagram(data, QHostAddress("192.168.4.20"), 5002);
    qDebug() << "[VinManager] Sent bind to sensor module:" << QString::fromUtf8(data);
}

void VinManager::sendBindToBody(const QString &vinStr)
{
    QJsonObject cmd;
    cmd["cmd"] = "bind";
    cmd["vin"] = vinStr;
    QJsonDocument doc(cmd);
    QByteArray data = doc.toJson(QJsonDocument::Compact);

    if (m_bodySocket.state() == QAbstractSocket::ConnectedState) {
        m_bodySocket.write(data);
        m_bodySocket.write("\n");
        m_bodySocket.flush();
    } else {
        // Connect first, then send
        m_bodySocket.connectToHost(QHostAddress("192.168.4.1"), 5000);
        connect(&m_bodySocket, &QTcpSocket::connected, this, [this, data]() {
            m_bodySocket.write(data);
            m_bodySocket.write("\n");
            m_bodySocket.flush();
            disconnect(&m_bodySocket, &QTcpSocket::connected, nullptr, nullptr);
        });
    }
    qDebug() << "[VinManager] Sent bind to body controller:" << QString::fromUtf8(data);
}

void VinManager::sendUnbindToSensor()
{
    QJsonObject cmd;
    cmd["cmd"] = "unbind";
    QJsonDocument doc(cmd);
    QByteArray data = doc.toJson(QJsonDocument::Compact);
    m_sensorSocket.writeDatagram(data, QHostAddress("192.168.4.20"), 5002);
}

void VinManager::sendUnbindToBody()
{
    QJsonObject cmd;
    cmd["cmd"] = "unbind";
    QJsonDocument doc(cmd);
    QByteArray data = doc.toJson(QJsonDocument::Compact);

    if (m_bodySocket.state() == QAbstractSocket::ConnectedState) {
        m_bodySocket.write(data);
        m_bodySocket.write("\n");
        m_bodySocket.flush();
    }
}

// --- Network receive ---

void VinManager::onSensorBindReply()
{
    while (m_sensorSocket.hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(m_sensorSocket.pendingDatagramSize());
        QHostAddress sender;
        quint16 senderPort;
        m_sensorSocket.readDatagram(datagram.data(), datagram.size(), &sender, &senderPort);

        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson(datagram, &parseError);
        if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
            qDebug() << "[VinManager] Sensor reply parse error:" << parseError.errorString();
            continue;
        }

        QJsonObject obj = doc.object();
        QString event = obj["event"].toString();

        if (event == "bound") {
            m_sensorModuleId = obj["moduleId"].toString();
            m_sensorMac = obj["mac"].toString();
            m_sensorBound = true;
            m_settings.setValue("sensor/bound", true);
            m_settings.setValue("sensor/moduleId", m_sensorModuleId);
            m_settings.setValue("sensor/mac", m_sensorMac);
            m_settings.sync();

            emit sensorBoundChanged();
            emit sensorModuleIdChanged();
            emit sensorMacChanged();

            setBusy(false);
            setStatus("Sensor bound: " + m_sensorModuleId);
            emit bindSucceeded("sensor", m_sensorModuleId);
            qDebug() << "[VinManager] Sensor bound:" << m_sensorModuleId << "MAC:" << m_sensorMac;
        } else if (event == "unbound") {
            m_sensorBound = false;
            m_sensorModuleId.clear();
            m_sensorMac.clear();
            saveConfig();
            emit sensorBoundChanged();
            emit sensorModuleIdChanged();
            emit sensorMacChanged();
        } else if (event == "bind_error") {
            setBusy(false);
            QString reason = obj["error"].toString();
            emit bindFailed("sensor", reason);
            setStatus("Sensor bind failed: " + reason);
        }
    }
}

void VinManager::onBodyBindReply()
{
    QByteArray data = m_bodySocket.readAll();
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);

    if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
        // Body controller might still be in text-only mode — look for text response
        QString text = QString::fromUtf8(data).trimmed();
        if (text.startsWith("BIND:")) {
            // Text protocol bind response: "BIND:BC-XXXXXXXX\n"
            QString moduleId = text.mid(5);
            m_bodyModuleId = moduleId;
            m_bodyBound = true;
            m_bodyMac = "00:00:00:00:00:00"; // Text protocol doesn't return MAC
            saveConfig();
            emit bodyBoundChanged();
            emit bodyModuleIdChanged();
            emit bodyMacChanged();
            setBusy(false);
            setStatus("Body bound: " + m_bodyModuleId);
            emit bindSucceeded("body", m_bodyModuleId);
        }
        return;
    }

    QJsonObject obj = doc.object();
    QString event = obj["event"].toString();

    if (event == "bound") {
        m_bodyModuleId = obj["moduleId"].toString();
        m_bodyMac = obj["mac"].toString();
        m_bodyBound = true;
        m_settings.setValue("body/bound", true);
        m_settings.setValue("body/moduleId", m_bodyModuleId);
        m_settings.setValue("body/mac", m_bodyMac);
        m_settings.sync();

        emit bodyBoundChanged();
        emit bodyModuleIdChanged();
        emit bodyMacChanged();

        setBusy(false);
        setStatus("Body bound: " + m_bodyModuleId);
        emit bindSucceeded("body", m_bodyModuleId);
        qDebug() << "[VinManager] Body bound:" << m_bodyModuleId << "MAC:" << m_bodyMac;
    } else if (event == "unbound") {
        m_bodyBound = false;
        m_bodyModuleId.clear();
        m_bodyMac.clear();
        saveConfig();
        emit bodyBoundChanged();
        emit bodyModuleIdChanged();
        emit bodyMacChanged();
    } else if (event == "bind_error") {
        setBusy(false);
        QString reason = obj["error"].toString();
        emit bindFailed("body", reason);
        setStatus("Body bind failed: " + reason);
    }
}

// --- Helpers ---

void VinManager::setStatus(const QString &msg)
{
    m_statusText = msg;
    emit statusTextChanged();
    m_statusTimer.start();
}

void VinManager::setBusy(bool busy)
{
    if (m_busy == busy)
        return;
    m_busy = busy;
    emit busyChanged();
}
