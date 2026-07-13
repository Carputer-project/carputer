#ifndef ALARMMANAGER_H
#define ALARMMANAGER_H

#include <QObject>
#include <QSettings>
#include <QTimer>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QTcpSocket>
#include <QHostAddress>

struct AlarmEvent {
    QString timestamp;
    QString trigger;   // "door_open", "hood_open", "ignition_no_unlock", "vibration"
    QString severity;  // "low", "medium", "high", "critical"
    QString source;    // "driver_door", "passenger_door", "trunk", "hood"
};

class AlarmManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool armed READ armed NOTIFY armedChanged)
    Q_PROPERTY(bool triggered READ triggered NOTIFY triggeredChanged)
    Q_PROPERTY(QString alarmState READ alarmState NOTIFY alarmStateChanged)
    Q_PROPERTY(QString statusText READ statusText NOTIFY statusTextChanged)
    Q_PROPERTY(int eventCount READ eventCount NOTIFY eventCountChanged)
    Q_PROPERTY(bool sirenOn READ sirenOn NOTIFY sirenOnChanged)
    Q_PROPERTY(bool busy READ busy NOTIFY busyChanged)

public:
    explicit AlarmManager(QObject *parent = nullptr);

    bool armed() const { return m_armed; }
    bool triggered() const { return m_triggered; }
    QString alarmState() const { return m_alarmState; }
    QString statusText() const { return m_statusText; }
    int eventCount() const { return m_events.size(); }
    bool sirenOn() const { return m_sirenOn; }
    bool busy() const { return m_busy; }

    Q_INVOKABLE void arm();
    Q_INVOKABLE void disarm();
    Q_INVOKABLE void clearEvents();
    Q_INVOKABLE QString eventTimestamp(int index) const;
    Q_INVOKABLE QString eventTrigger(int index) const;
    Q_INVOKABLE QString eventSeverity(int index) const;
    Q_INVOKABLE QString eventSource(int index) const;

signals:
    void armedChanged();
    void triggeredChanged();
    void alarmStateChanged();
    void statusTextChanged();
    void eventCountChanged();
    void sirenOnChanged();
    void busyChanged();
    void alarmEvent(const QString &trigger, const QString &severity, const QString &source);

private slots:
    void onStatusTimer();
    void onTcpConnected();
    void onTcpReadyRead();
    void onTcpError(QAbstractSocket::SocketError error);

private:
    void setStatus(const QString &msg);
    void setBusy(bool busy);
    void loadEvents();
    void saveEvents();
    void sendCommand(const QString &cmd);
    void parseStatusLine(const QString &line);

    QSettings m_settings;
    QTcpSocket m_socket;
    QTimer m_statusTimer;
    QTimer m_reconnectTimer;

    bool m_armed;
    bool m_triggered;
    bool m_sirenOn;
    QString m_alarmState;
    QString m_statusText;
    bool m_busy;

    QList<AlarmEvent> m_events;
    QString m_pendingCmd;
};

#endif // ALARMMANAGER_H
