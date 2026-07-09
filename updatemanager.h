#pragma once
#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QProcess>
#include <QTimer>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(lcUpdate)

class UpdateManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString status         READ status         NOTIFY statusChanged)
    Q_PROPERTY(QString currentVersion READ currentVersion NOTIFY currentVersionChanged)
    Q_PROPERTY(QString serverVersion  READ serverVersion  NOTIFY serverVersionChanged)
    Q_PROPERTY(bool    updateAvailable  READ updateAvailable  NOTIFY updateAvailableChanged)
    Q_PROPERTY(bool    usbUpdateFound   READ usbUpdateFound   NOTIFY usbUpdateFoundChanged)
    Q_PROPERTY(QString usbPath        READ usbPath        NOTIFY usbPathChanged)
    Q_PROPERTY(bool    busy           READ busy           NOTIFY busyChanged)
    Q_PROPERTY(int     progress       READ progress       NOTIFY progressChanged)

public:
    explicit UpdateManager(QObject *parent = nullptr);
    ~UpdateManager() override;

    QString status()          const { return m_status; }
    QString currentVersion()  const { return m_currentVersion; }
    QString serverVersion()   const { return m_serverVersion; }
    bool    updateAvailable() const { return m_updateAvailable; }
    bool    usbUpdateFound()  const { return m_usbUpdateFound; }
    QString usbPath()         const { return m_usbPath; }
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
    void updateAvailableChanged();
    void usbUpdateFoundChanged();
    void usbPathChanged();
    void busyChanged();
    void progressChanged();
    void updateComplete(bool success, const QString &message);

private slots:
    void onGitHubReplyFinished();
    void onWgetFinished();
    void scanUsbDrives();

private:
    void setStatus(const QString &s);
    void setBusy(bool b);
    void setProgress(int p);
    bool applyPackage(const QString &packagePath);
    QString readCurrentVersion();
    static bool isNewerVersion(const QString &current, const QString &latest);

    QNetworkAccessManager *m_nam       = nullptr;
    QNetworkReply         *m_reply     = nullptr;
    QProcess              *m_wget      = nullptr;
    QTimer                *m_usbTimer  = nullptr;

    QString  m_status;
    QString  m_currentVersion;
    QString  m_serverVersion;
    QString  m_usbPath;
    QString  m_downloadUrl;

    bool  m_updateAvailable = false;
    bool  m_usbUpdateFound  = false;
    bool  m_busy            = false;
    int   m_progress        = 0;

    static constexpr int  USB_CHECK_MS   = 3000;

    static constexpr char PACKAGE_NAME[] = "carputer_update.tar.gz";
    static constexpr char VERSION_FILE[] = "/etc/carputer/version.txt";
    static constexpr char BINARY_DEST[]  = "/usr/bin/carputer";
    static constexpr char STAGING_DIR[]  = "/tmp/carputer_update_stage";
    static constexpr char DOWNLOAD_TMP[] = "/tmp/carputer_update.tar.gz";
    static constexpr char GITHUB_API[]   = "https://api.github.com/repos/Kingfrezz/carputer/releases/latest";
    static constexpr char USER_AGENT[]   = "Carputer/1.0";
};
