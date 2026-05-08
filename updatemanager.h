#pragma once
#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTimer>
#include <QUdpSocket>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(lcUpdate)

// ── UpdateManager ─────────────────────────────────────────────────────────────
// Handles Carputer OTA updates from two sources:
//   1. Network  — UDP discovery finds a Windows/Linux server on the LAN;
//                 app downloads carputer_update.tar.gz over HTTP
//   2. USB drive — package auto-detected at root of any mounted volume
//
// Package format (carputer_update.tar.gz flat contents):
//   carputer      — compiled binary  → installed to /usr/bin/carputer
//   version.txt   — "1.2.3\n"       → installed to /etc/carputer/version.txt
//
// The app reads its current version from /etc/carputer/version.txt at startup.
// applyPackage() uses a staging dir so each file lands in the right location.
// ─────────────────────────────────────────────────────────────────────────────

class UpdateManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString status         READ status         NOTIFY statusChanged)
    Q_PROPERTY(QString currentVersion READ currentVersion NOTIFY currentVersionChanged)
    Q_PROPERTY(QString serverVersion  READ serverVersion  NOTIFY serverVersionChanged)
    Q_PROPERTY(QString serverUrl      READ serverUrl      NOTIFY serverUrlChanged)
    Q_PROPERTY(QString usbPath        READ usbPath        NOTIFY usbPathChanged)
    Q_PROPERTY(bool    updateAvailable  READ updateAvailable  NOTIFY updateAvailableChanged)
    Q_PROPERTY(bool    usbUpdateFound   READ usbUpdateFound   NOTIFY usbUpdateFoundChanged)
    Q_PROPERTY(bool    busy           READ busy           NOTIFY busyChanged)
    Q_PROPERTY(int     progress       READ progress       NOTIFY progressChanged)

public:
    explicit UpdateManager(QObject *parent = nullptr);
    ~UpdateManager() override;

    QString status()          const { return m_status; }
    QString currentVersion()  const { return m_currentVersion; }
    QString serverVersion()   const { return m_serverVersion; }
    QString serverUrl()       const { return m_serverUrl; }
    QString usbPath()         const { return m_usbPath; }
    bool    updateAvailable() const { return m_updateAvailable; }
    bool    usbUpdateFound()  const { return m_usbUpdateFound; }
    bool    busy()            const { return m_busy; }
    int     progress()        const { return m_progress; }

    Q_INVOKABLE void checkForUpdate();
    Q_INVOKABLE void checkUsb();
    Q_INVOKABLE void applyNetworkUpdate();
    Q_INVOKABLE void applyUsbUpdate();

signals:
    void statusChanged();
    void currentVersionChanged();
    void serverVersionChanged();
    void serverUrlChanged();
    void usbPathChanged();
    void updateAvailableChanged();
    void usbUpdateFoundChanged();
    void busyChanged();
    void progressChanged();
    void updateComplete(bool success, const QString &message);

private slots:
    void onDiscoveryResponse();
    void onDownloadProgress(qint64 received, qint64 total);
    void onDownloadFinished();
    void scanUsbDrives();

private:
    void setStatus(const QString &s);
    void setBusy(bool b);
    void setProgress(int p);
    bool applyPackage(const QString &packagePath);
    QString readCurrentVersion();
    void discoverServer();

    QNetworkAccessManager *m_nam       = nullptr;
    QNetworkReply         *m_reply     = nullptr;
    QUdpSocket            *m_udpSocket = nullptr;
    QTimer                *m_usbTimer  = nullptr;

    QString  m_status;
    QString  m_currentVersion;
    QString  m_serverVersion;
    QString  m_serverUrl;
    QString  m_usbPath;

    bool  m_updateAvailable = false;
    bool  m_usbUpdateFound  = false;
    bool  m_busy            = false;
    int   m_progress        = 0;

    // ── Constants ─────────────────────────────────────────────────────────────
    static constexpr int  DISCOVERY_PORT = 42424;
    static constexpr int  SERVER_PORT    = 42425;
    static constexpr int  USB_CHECK_MS   = 3000;

    // Package and install paths
    static constexpr char PACKAGE_NAME[] = "carputer_update.tar.gz";
    static constexpr char VERSION_FILE[] = "/etc/carputer/version.txt";
    static constexpr char BINARY_DEST[]  = "/usr/bin/carputer";
    static constexpr char STAGING_DIR[]  = "/tmp/carputer_update_stage";
    static constexpr char DOWNLOAD_TMP[] = "/tmp/carputer_update.tar.gz";
};
