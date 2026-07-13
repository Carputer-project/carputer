#ifndef REMOTEKEYMANAGER_H
#define REMOTEKEYMANAGER_H

#include <QObject>
#include <QTcpSocket>
#include <QHostAddress>
#include <QTimer>
#include <QJsonObject>
#include <QJsonDocument>
#include <QCryptographicHash>

class RemoteKeyManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool connected READ connected NOTIFY connectedChanged)
    Q_PROPERTY(bool locked READ locked NOTIFY lockedChanged)
    Q_PROPERTY(QString statusText READ statusText NOTIFY statusTextChanged)
    Q_PROPERTY(bool busy READ busy NOTIFY busyChanged)

public:
    explicit RemoteKeyManager(QObject *parent = nullptr);

    bool connected() const { return m_connected; }
    bool locked() const { return m_locked; }
    QString statusText() const { return m_statusText; }
    bool busy() const { return m_busy; }

    Q_INVOKABLE void lockDoors();
    Q_INVOKABLE void unlockDoors();
    Q_INVOKABLE void armAlarm();
    Q_INVOKABLE void disarmAlarm();
    Q_INVOKABLE void remoteStart();
    Q_INVOKABLE void remoteStop();

    void setSecret(const QString &secret) { m_secret = secret; }
    void setVin(const QString &vin) { m_vin = vin; }

signals:
    void connectedChanged();
    void lockedChanged();
    void statusTextChanged();
    void busyChanged();
    void commandAck(const QString &cmd);
    void commandNack(const QString &cmd, const QString &reason);

private slots:
    void onTcpConnected();
    void onTcpDisconnected();
    void onTcpReadyRead();
    void onTcpError(QAbstractSocket::SocketError error);

private:
    void setStatus(const QString &msg);
    void setBusy(bool busy);
    void sendAuthenticatedCommand(const QString &cmd);
    QString computeHmac(const QString &data);

    QTcpSocket m_socket;
    QTimer m_reconnectTimer;

    bool m_connected;
    bool m_locked;
    bool m_busy;
    QString m_statusText;

    QString m_secret;
    QString m_vin;
    quint32 m_nonce;
};

#endif // REMOTEKEYMANAGER_H
