#ifndef INTERNALWIFIMANAGER_H
#define INTERNALWIFIMANAGER_H

#include <QObject>
#include <QProcess>
#include <QString>
#include <QStringList>
#include <QTimer>

class InternalWiFiManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool connected READ connected NOTIFY connectedChanged)
    Q_PROPERTY(QString ssid READ ssid NOTIFY ssidChanged)
    Q_PROPERTY(QString ipAddress READ ipAddress NOTIFY ipAddressChanged)
    Q_PROPERTY(int signalStrength READ signalStrength NOTIFY signalStrengthChanged())
    Q_PROPERTY(QString statusText READ statusText NOTIFY statusTextChanged())
    Q_PROPERTY(QStringList networks READ networks NOTIFY networksChanged())

public:
    explicit InternalWiFiManager(QObject *parent = nullptr);
    ~InternalWiFiManager() override;

    bool connected() const { return m_connected; }
    QString ssid() const { return m_ssid; }
    QString ipAddress() const { return m_ipAddress; }
    int signalStrength() const { return m_signalStrength; }
    QString statusText() const { return m_statusText; }
    QStringList networks() const { return m_networks; }

    Q_INVOKABLE void scanNetworks();
    Q_INVOKABLE void connectToNetwork(const QString &ssid, const QString &password = QString());
    Q_INVOKABLE void disconnectNetwork();
    Q_INVOKABLE void forgetNetwork(const QString &ssid);
    Q_INVOKABLE void setStaticIP(const QString &ip, const QString &netmask = "255.255.255.0", const QString &gateway = "");
    Q_INVOKABLE void connectToCarputerECU();

signals:
    void connectedChanged();
    void ssidChanged();
    void ipAddressChanged();
    void signalStrengthChanged();
    void statusTextChanged();
    void networksChanged();
    void connectionFailed(const QString &error);
    void passwordRequired(const QString &ssid);

private slots:
    void onScanFinished();
    void onConnectFinished();
    void onDisconnectFinished();
    void checkConnectionStatus();

private:
    void setConnected(bool c);
    void setSsid(const QString &s);
    void setIpAddress(const QString &ip);
    void setSignalStrength(int strength);
    void setStatusText(const QString &text);
    void updateNetworks(const QStringList &networks);

    QProcess m_scanProcess;
    QProcess m_connectProcess;
    QProcess m_statusProcess;
    QString m_interface = "wlp3s0b1";
    bool m_connected = false;
    QString m_ssid;
    QString m_ipAddress;
    int m_signalStrength = 0;
    QString m_statusText = "Not connected";
    QStringList m_networks;
    QTimer *m_statusTimer = nullptr;
    int m_connectRetryCount = 0;
};

#endif // INTERNALWIFIMANAGER_H
