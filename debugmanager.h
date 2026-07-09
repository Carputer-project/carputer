#ifndef DEBUGMANAGER_H
#define DEBUGMANAGER_H

#include <QObject>
#include <QTimer>
#include <QFile>
#include <QTextStream>

class SensorManager;
class CarControlManager;
class InternalWiFiManager;
class MediaManager;
class CameraManager;
class AudioManager;

class DebugManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool running READ running NOTIFY runningChanged)
    Q_PROPERTY(QString lastReport READ lastReport NOTIFY reportUpdated)
    Q_PROPERTY(int issueCount READ issueCount NOTIFY reportUpdated)

public:
    explicit DebugManager(QObject *parent = nullptr);
    ~DebugManager() override;

    bool running() const { return m_running; }
    QString lastReport() const { return m_lastReport; }
    int issueCount() const { return m_issues.count(); }

    // Manager references (set by main.cpp)
    void setSensorManager(SensorManager *mgr) { m_sensorManager = mgr; }
    void setCarControlManager(CarControlManager *mgr) { m_carControlManager = mgr; }
    void setInternalWiFiManager(InternalWiFiManager *mgr) { m_internalWiFiManager = mgr; }
    void setMediaManager(MediaManager *mgr) { m_mediaManager = mgr; }
    void setCameraManager(CameraManager *mgr) { m_cameraManager = mgr; }
    void setAudioManager(AudioManager *mgr) { m_audioManager = mgr; }

    Q_INVOKABLE void runDiagnostics();
    Q_INVOKABLE void saveReport(const QString &path = "/root/carputer_debug_report.txt");

signals:
    void runningChanged();
    void reportUpdated();
    void issueFound(const QString &component, const QString &issue);

private slots:
    void checkTimers();

private:
    void checkSensorManager();
    void checkCarControlManager();
    void checkInternalWiFi();
    void checkMediaManager();
    void checkCameraManager();
    void checkAudioManager();
    void checkGStreamerPlugins();
    void addIssue(const QString &component, const QString &issue);
    void generateReport();

    bool m_running = false;
    QString m_lastReport;
    QStringList m_issues;

    SensorManager *m_sensorManager = nullptr;
    CarControlManager *m_carControlManager = nullptr;
    InternalWiFiManager *m_internalWiFiManager = nullptr;
    MediaManager *m_mediaManager = nullptr;
    CameraManager *m_cameraManager = nullptr;
    AudioManager *m_audioManager = nullptr;

    QTimer *m_checkTimer = nullptr;
};

#endif // DEBUGMANAGER_H
