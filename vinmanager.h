#ifndef VINMANAGER_H
#define VINMANAGER_H

#include <QObject>
#include <QSettings>
#include <QUdpSocket>
#include <QTcpSocket>
#include <QTimer>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QHostAddress>
#include <QCryptographicHash>

class VinManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString vin READ vin WRITE setVin NOTIFY vinChanged)
    Q_PROPERTY(QString secret READ secret NOTIFY secretChanged)

    // Body Controller
    Q_PROPERTY(bool bodyBound READ bodyBound NOTIFY bodyBoundChanged)
    Q_PROPERTY(QString bodyModuleId READ bodyModuleId NOTIFY bodyModuleIdChanged)
    Q_PROPERTY(QString bodyMac READ bodyMac NOTIFY bodyMacChanged)

    // Sensor Module
    Q_PROPERTY(bool sensorBound READ sensorBound NOTIFY sensorBoundChanged)
    Q_PROPERTY(QString sensorModuleId READ sensorModuleId NOTIFY sensorModuleIdChanged)
    Q_PROPERTY(QString sensorMac READ sensorMac NOTIFY sensorMacChanged)

    // Mesh Module
    Q_PROPERTY(bool meshBound READ meshBound NOTIFY meshBoundChanged)
    Q_PROPERTY(QString meshModuleId READ meshModuleId NOTIFY meshModuleIdChanged)
    Q_PROPERTY(QString meshMac READ meshMac NOTIFY meshMacChanged)

    // Remote Fob
    Q_PROPERTY(bool remoteBound READ remoteBound NOTIFY remoteBoundChanged)
    Q_PROPERTY(QString remoteModuleId READ remoteModuleId NOTIFY remoteModuleIdChanged)
    Q_PROPERTY(QString remoteMac READ remoteMac NOTIFY remoteMacChanged)

    Q_PROPERTY(QString statusText READ statusText NOTIFY statusTextChanged)
    Q_PROPERTY(bool busy READ busy NOTIFY busyChanged)

public:
    explicit VinManager(QObject *parent = nullptr);

    // Getters
    QString vin() const { return m_vin; }
    QString secret() const { return m_secret; }
    bool bodyBound() const { return m_bodyBound; }
    QString bodyModuleId() const { return m_bodyModuleId; }
    QString bodyMac() const { return m_bodyMac; }
    bool sensorBound() const { return m_sensorBound; }
    QString sensorModuleId() const { return m_sensorModuleId; }
    QString sensorMac() const { return m_sensorMac; }
    bool meshBound() const { return m_meshBound; }
    QString meshModuleId() const { return m_meshModuleId; }
    QString meshMac() const { return m_meshMac; }
    bool remoteBound() const { return m_remoteBound; }
    QString remoteModuleId() const { return m_remoteModuleId; }
    QString remoteMac() const { return m_remoteMac; }
    QString statusText() const { return m_statusText; }
    bool busy() const { return m_busy; }

    Q_INVOKABLE void setVin(const QString &vin);

    // Bind/unbind actions — send commands to ESP32 modules
    Q_INVOKABLE void bindBody();
    Q_INVOKABLE void bindSensor();
    Q_INVOKABLE void bindMesh();
    Q_INVOKABLE void bindRemote();
    Q_INVOKABLE void unbindBody();
    Q_INVOKABLE void unbindSensor();
    Q_INVOKABLE void unbindMesh();
    Q_INVOKABLE void unbindRemote();
    Q_INVOKABLE void unbindAll();

    // Utility
    Q_INVOKABLE QString generateModuleId(const QString &prefix, const QString &mac, const QString &vinStr);

signals:
    void vinChanged();
    void secretChanged();
    void bodyBoundChanged();
    void bodyModuleIdChanged();
    void bodyMacChanged();
    void sensorBoundChanged();
    void sensorModuleIdChanged();
    void sensorMacChanged();
    void meshBoundChanged();
    void meshModuleIdChanged();
    void meshMacChanged();
    void remoteBoundChanged();
    void remoteModuleIdChanged();
    void remoteMacChanged();
    void statusTextChanged();
    void busyChanged();
    void bindSucceeded(const QString &moduleType, const QString &moduleId);
    void bindFailed(const QString &moduleType, const QString &reason);
    void unbindSucceeded(const QString &moduleType);
    void allUnbound();

private slots:
    void onSensorBindReply();
    void onBodyBindReply();

private:
    void loadConfig();
    void saveConfig();
    void generateSecret();
    void setStatus(const QString &msg);
    void setBusy(bool busy);
    void sendBindToSensor(const QString &vinStr);
    void sendBindToBody(const QString &vinStr);
    void sendUnbindToSensor();
    void sendUnbindToBody();

    QSettings m_settings;
    QUdpSocket m_sensorSocket;
    QTcpSocket m_bodySocket;
    QTimer m_statusTimer;

    QString m_vin;
    QString m_secret;

    // Body Controller
    bool m_bodyBound;
    QString m_bodyModuleId;
    QString m_bodyMac;

    // Sensor Module
    bool m_sensorBound;
    QString m_sensorModuleId;
    QString m_sensorMac;

    // Mesh Module
    bool m_meshBound;
    QString m_meshModuleId;
    QString m_meshMac;

    // Remote Fob
    bool m_remoteBound;
    QString m_remoteModuleId;
    QString m_remoteMac;

    QString m_statusText;
    bool m_busy;

    // Pending bind state
    QString m_pendingBindType;
    QString m_pendingBindVin;
};

#endif // VINMANAGER_H
