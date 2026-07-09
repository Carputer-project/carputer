#ifndef DTCMANAGER_H
#define DTCMANAGER_H

#include <QObject>
#include <QUdpSocket>
#include <QVariantList>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QTimer>
#include <QDateTime>

class DtcManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QVariantList dtcCodes READ dtcCodes NOTIFY dtcUpdated)
    Q_PROPERTY(int dtcCount READ dtcCount NOTIFY dtcUpdated)
    Q_PROPERTY(QVariantList dtcHistory READ dtcHistory NOTIFY dtcHistoryChanged)
    Q_PROPERTY(bool busy READ busy NOTIFY busyChanged)
    Q_PROPERTY(QString statusText READ statusText NOTIFY statusTextChanged)
    Q_PROPERTY(QString lastScanMode READ lastScanMode NOTIFY dtcUpdated)

public:
    explicit DtcManager(QObject *parent = nullptr);
    ~DtcManager() override;

    QVariantList dtcCodes() const { return m_dtcCodes; }
    int dtcCount() const { return m_dtcCount; }
    QVariantList dtcHistory() const { return m_dtcHistory; }
    bool busy() const { return m_busy; }
    QString statusText() const { return m_statusText; }
    QString lastScanMode() const { return m_lastScanMode; }

    Q_INVOKABLE void scanDtc();
    Q_INVOKABLE void scanDtcTestMode();
    Q_INVOKABLE void clearHistory();

    // DTC description lookup
    Q_INVOKABLE QString describeCode(int code) const;

signals:
    void dtcUpdated();
    void dtcHistoryChanged();
    void busyChanged();
    void statusTextChanged();

private slots:
    void onReadyRead();

private:
    void sendCommand(const QString &cmd);
    void setBusy(bool b);
    void setStatus(const QString &text);
    void loadHistory();
    void saveHistory();

    QUdpSocket *m_socket = nullptr;
    QVariantList m_dtcCodes;
    int m_dtcCount = 0;
    QVariantList m_dtcHistory;
    bool m_busy = false;
    QString m_statusText = QStringLiteral("Ready");
    QString m_lastScanMode;
    QTimer *m_timeoutTimer = nullptr;
};

#endif // DTCMANAGER_H
