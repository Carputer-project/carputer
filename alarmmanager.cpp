#include "alarmmanager.h"
#include <QDebug>
#include <QDateTime>

AlarmManager::AlarmManager(QObject *parent)
    : QObject(parent)
    , m_settings("/etc/carputer/alarm.conf", QSettings::IniFormat)
    , m_armed(false)
    , m_triggered(false)
    , m_sirenOn(false)
    , m_alarmState("disarmed")
    , m_busy(false)
{
    loadEvents();

    m_statusTimer.setSingleShot(true);
    m_statusTimer.setInterval(5000);
    connect(&m_statusTimer, &QTimer::timeout, this, &AlarmManager::onStatusTimer);

    connect(&m_socket, &QTcpSocket::connected, this, &AlarmManager::onTcpConnected);
    connect(&m_socket, &QTcpSocket::readyRead, this, &AlarmManager::onTcpReadyRead);
    connect(&m_socket, &QTcpSocket::errorOccurred, this, &AlarmManager::onTcpError);

    // Auto-reconnect every 10 seconds if disconnected
    m_reconnectTimer.setInterval(10000);
    connect(&m_reconnectTimer, &QTimer::timeout, this, [this]() {
        if (m_socket.state() == QAbstractSocket::UnconnectedState) {
            m_socket.connectToHost(QHostAddress("192.168.4.1"), 5000);
        }
    });
    m_reconnectTimer.start();

    // Connect on startup
    m_socket.connectToHost(QHostAddress("192.168.4.1"), 5000);
}

void AlarmManager::arm()
{
    if (m_armed) {
        setStatus("Already armed");
        return;
    }
    setStatus("Arming...");
    setBusy(true);
    m_pendingCmd = "arm";
    sendCommand("arm");
}

void AlarmManager::disarm()
{
    if (!m_armed && !m_triggered) {
        setStatus("Already disarmed");
        return;
    }
    setStatus("Disarming...");
    setBusy(true);
    m_pendingCmd = "disarm";
    sendCommand("disarm");
}

void AlarmManager::clearEvents()
{
    m_events.clear();
    saveEvents();
    emit eventCountChanged();
    setStatus("Events cleared");
}

QString AlarmManager::eventTimestamp(int index) const
{
    if (index < 0 || index >= m_events.size()) return {};
    return m_events[index].timestamp;
}

QString AlarmManager::eventTrigger(int index) const
{
    if (index < 0 || index >= m_events.size()) return {};
    return m_events[index].trigger;
}

QString AlarmManager::eventSeverity(int index) const
{
    if (index < 0 || index >= m_events.size()) return {};
    return m_events[index].severity;
}

QString AlarmManager::eventSource(int index) const
{
    if (index < 0 || index >= m_events.size()) return {};
    return m_events[index].source;
}

void AlarmManager::loadEvents()
{
    int count = m_settings.value("eventCount", 0).toInt();
    for (int i = 0; i < count && i < 100; i++) {
        AlarmEvent ev;
        ev.timestamp = m_settings.value(QString("event%1/timestamp").arg(i)).toString();
        ev.trigger = m_settings.value(QString("event%1/trigger").arg(i)).toString();
        ev.severity = m_settings.value(QString("event%1/severity").arg(i)).toString();
        ev.source = m_settings.value(QString("event%1/source").arg(i)).toString();
        if (!ev.timestamp.isEmpty())
            m_events.append(ev);
    }
}

void AlarmManager::saveEvents()
{
    m_settings.remove("eventCount");
    for (int i = 0; i < m_events.size(); i++) {
        m_settings.setValue(QString("event%1/timestamp").arg(i), m_events[i].timestamp);
        m_settings.setValue(QString("event%1/trigger").arg(i), m_events[i].trigger);
        m_settings.setValue(QString("event%1/severity").arg(i), m_events[i].severity);
        m_settings.setValue(QString("event%1/source").arg(i), m_events[i].source);
    }
    m_settings.setValue("eventCount", m_events.size());
    m_settings.sync();
}

void AlarmManager::sendCommand(const QString &cmd)
{
    if (m_socket.state() == QAbstractSocket::ConnectedState) {
        QByteArray data = ("{\"cmd\":\"" + cmd + "\"}\n").toUtf8();
        m_socket.write(data);
        m_socket.flush();
    } else {
        setStatus("Not connected to body controller");
        setBusy(false);
    }
}

void AlarmManager::parseStatusLine(const QString &line)
{
    // Parse: "H:0 S:0 A:0 L:0 R:0 F:0 P:0,0 K:0 V:0 M:1 B:16 T:16 D:0 O:0"
    // The alarm state isn't in the standard status — we get it from JSON events
    // For now, track armed/disarmed state from our own commands
}

void AlarmManager::onStatusTimer()
{
    m_statusText.clear();
    emit statusTextChanged();
}

void AlarmManager::onTcpConnected()
{
    qDebug() << "[AlarmManager] Connected to body controller";
    setStatus("Connected");
}

void AlarmManager::onTcpReadyRead()
{
    QByteArray data = m_socket.readAll();
    QString text = QString::fromUtf8(data).trimmed();

    // Check for JSON alarm events
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isObject()) {
        QJsonObject obj = doc.object();
        QString event = obj["event"].toString();

        if (event == "alarm") {
            QString trigger = obj["trigger"].toString();
            QString severity = obj["severity"].toString();
            QString source = obj["source"].toString();
            bool armed = obj["armed"].toBool();

            m_armed = armed;
            emit armedChanged();

            if (trigger == "armed") {
                m_alarmState = "armed";
                m_triggered = false;
                emit alarmStateChanged();
                emit triggeredChanged();
                setBusy(false);
                setStatus("Armed");
            } else if (trigger == "disarmed") {
                m_alarmState = "disarmed";
                m_triggered = false;
                m_sirenOn = false;
                emit alarmStateChanged();
                emit triggeredChanged();
                emit sirenOnChanged();
                setBusy(false);
                setStatus("Disarmed");
            } else if (trigger == "triggered") {
                m_alarmState = "triggered";
                m_triggered = true;
                m_sirenOn = true;
                emit alarmStateChanged();
                emit triggeredChanged();
                emit sirenOnChanged();
                setStatus("ALARM TRIGGERED: " + source);

                // Log event
                AlarmEvent ev;
                ev.timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
                ev.trigger = obj["triggerType"].toString();
                ev.severity = severity;
                ev.source = source;
                m_events.prepend(ev);
                if (m_events.size() > 100) m_events.removeLast();
                saveEvents();
                emit eventCountChanged();
                emit alarmEvent(ev.trigger, ev.severity, ev.source);
            }
        }
    }
}

void AlarmManager::onTcpError(QAbstractSocket::SocketError error)
{
    Q_UNUSED(error)
    if (m_busy) {
        setBusy(false);
        setStatus("Connection failed");
    }
}

void AlarmManager::setStatus(const QString &msg)
{
    m_statusText = msg;
    emit statusTextChanged();
    m_statusTimer.start();
}

void AlarmManager::setBusy(bool busy)
{
    if (m_busy == busy) return;
    m_busy = busy;
    emit busyChanged();
}
