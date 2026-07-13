#ifndef V2VMANAGER_H
#define V2VMANAGER_H

#include <QObject>
#include <QUdpSocket>
#include <QTimer>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QHostAddress>

struct MeshCarInfo {
    QString vin;
    QString moduleId;
    int hop;
    unsigned long lastSeen;
    int speed;
    int rpm;
    int gear;
    int coolant;
    int throttle;
};

class V2vManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int carCount READ carCount NOTIFY carCountChanged)
    Q_PROPERTY(bool meshActive READ meshActive NOTIFY meshActiveChanged)
    Q_PROPERTY(QString statusText READ statusText NOTIFY statusTextChanged)

public:
    explicit V2vManager(QObject *parent = nullptr);

    int carCount() const { return m_cars.size(); }
    bool meshActive() const { return m_meshActive; }
    QString statusText() const { return m_statusText; }

    Q_INVOKABLE QString carVin(int index) const;
    Q_INVOKABLE int carSpeed(int index) const;
    Q_INVOKABLE int carRpm(int index) const;
    Q_INVOKABLE int carGear(int index) const;
    Q_INVOKABLE int carHop(int index) const;
    Q_INVOKABLE int carCoolant(int index) const;
    Q_INVOKABLE int carThrottle(int index) const;
    Q_INVOKABLE QString carFreshness(int index) const;

signals:
    void carCountChanged();
    void meshActiveChanged();
    void statusTextChanged();
    void carDiscovered(const QString &vin, int speed, int rpm);
    void carLost(const QString &vin);
    void alertReceived(const QString &vin, const QString &alertType);

private slots:
    void onUdpReadyRead();
    void onExpireTimer();

private:
    void processPacket(const QByteArray &data);

    QUdpSocket m_socket;
    QTimer m_expireTimer;
    QList<MeshCarInfo> m_cars;
    bool m_meshActive;
    QString m_statusText;
    unsigned long m_lastPacketTime;
};

#endif // V2VMANAGER_H
