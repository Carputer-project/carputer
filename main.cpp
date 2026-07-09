#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQmlDebuggingEnabler>
#include <QScreen>
#include <QFileInfo>
#include <QUrl>
#include <QLoggingCategory>
#include <QWindow>
#include <QFile>
#include <QTextStream>

#include "systemmanager.h"
#include "configmanager.h"
#include "thememanager.h"
#include "diagnosticmanager.h"
#include "remotemanager.h"
#include "updatemanager.h"
#include "mediamanager.h"
#include "artworkprovider.h"
#include "videoframeprovider.h"
#include "dvrmanager.h"
#include "carplaymanager.h"
#include "carcontrolmanager.h"
#include "cameramanager.h"
#include "audiomanager.h"
#include "sensormanager.h"
#include "internalwifimanager.h"
#include "debugmanager.h"
#include "installmanager.h"
#include "dtcmanager.h"
#include "tripcomputer.h"
#include "datalogger.h"
#include "engineprofilemanager.h"

static QQmlDebuggingEnabler s_qmlDebuggingEnabler;

Q_LOGGING_CATEGORY(lcCarputer, "carputer.main")
Q_LOGGING_CATEGORY(lcQml,     "carputer.qml")

static void carputerMessageHandler(QtMsgType type,
                             const QMessageLogContext &ctx,
                             const QString &msg)
{
    const QString line = qFormatLogMessage(type, ctx, msg);
    fprintf(stderr, "%s\n", qPrintable(line));
    fflush(stderr);

    if (DiagnosticManager *dm = DiagnosticManager::instance())
        dm->appendLogLine(line);
}

#define QML_DISK_DIR  "/root/carputer"
#define QML_DISK_MAIN "/root/carputer/main.qml"

QString readVersionFromFile()
{
    QFile versionFile(QStringLiteral("/etc/carputer/version.txt"));
    if (versionFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&versionFile);
        QString version = in.readLine().trimmed();
        versionFile.close();
        if (!version.isEmpty()) {
            return version;
        }
    }
    return QStringLiteral("1.2"); // fallback default
}

int main(int argc, char *argv[])
{
    qSetMessagePattern("[%{time hh:mm:ss.zzz}] %{if-debug}D%{endif}"
                        "%{if-info}I%{endif}%{if-warning}W%{endif}"
                        "%{if-critical}C%{endif}%{if-fatal}F%{endif} "
                        "%{category} — %{message}");

    qInstallMessageHandler(carputerMessageHandler);

    if (qEnvironmentVariableIsEmpty("QT_QPA_PLATFORM")) {
        qputenv("QT_QPA_PLATFORM", "xcb");
    }

    QGuiApplication app(argc, argv);
    app.setApplicationName("Carputer");
    QString appVersion = readVersionFromFile();
    app.setApplicationVersion(appVersion);
    app.setOrganizationName("carputer");

    ThemeManager themeManager;
    ConfigManager configManager;

    // Initialize theme from configManager
    themeManager.setCurrentTheme(configManager.theme());
    if (!configManager.accentColor().isEmpty())
        themeManager.setAccentColor(QColor(configManager.accentColor()));
    themeManager.setGaugeBorderMode(configManager.gaugeBorderMode());

    QQmlApplicationEngine engine;

    engine.rootContext()->setContextProperty("themeManager", &themeManager);
    engine.rootContext()->setContextProperty("configManager", &configManager);

    SystemManager systemManager;
    engine.rootContext()->setContextProperty("systemManager", &systemManager);

    DiagnosticManager diagManager;
    engine.rootContext()->setContextProperty("diagnosticManager", &diagManager);

    // RemoteManager remoteManager;
    // engine.rootContext()->setContextProperty("remoteManager", &remoteManager);
    // Use configured port if set
    // if (!configManager.remotePort().isEmpty())
    //     remoteManager.setPort(configManager.remotePort());

    UpdateManager updateManager;
    engine.rootContext()->setContextProperty("updateManager", &updateManager);

    MediaManager mediaManager;
    engine.rootContext()->setContextProperty("mediaManager", &mediaManager);
    engine.addImageProvider(QStringLiteral("artwork"), mediaManager.artworkProvider());

    VideoFrameProvider videoFrameProvider;
    engine.addImageProvider(QStringLiteral("video"), &videoFrameProvider);

    DvrManager dvrManager;
    dvrManager.setVideoFrameProvider(&videoFrameProvider);
    engine.rootContext()->setContextProperty("dvrManager", &dvrManager);

    CarPlayManager carPlayManager;
    engine.rootContext()->setContextProperty("carPlayManager", &carPlayManager);

    CarControlManager carControlManager;
    engine.rootContext()->setContextProperty("carControlManager", &carControlManager);
    // Use configured port if set
    if (!configManager.carControlPort().isEmpty())
        carControlManager.setPort(configManager.carControlPort());

    CameraManager cameraManager;
    cameraManager.setVideoFrameProvider(&videoFrameProvider);
    engine.rootContext()->setContextProperty("cameraManager", &cameraManager);

    AudioManager audioManager;
    engine.rootContext()->setContextProperty("audioManager", &audioManager);
    audioManager.setMediaManager(&mediaManager);

    // ── Radio knob → media volume ────────────────────────────────────────────
    QObject::connect(&carControlManager, &CarControlManager::volumeSync,
                     &mediaManager, &MediaManager::setVolume);

    // ── Radio EQ knobs → media EQ ────────────────────────────────────────────
    // TEA6320T range 0-31 (16=flat) maps to GStreamer EQ -24..+12 (0=flat)
    auto mapRadioEq = [](int radioVal) -> double {
        return (radioVal - 16) * 0.75;
    };
    QObject::connect(&carControlManager, &CarControlManager::radioBassChanged,
                     &mediaManager, [&](int val) {
        mediaManager.setEqBand0(mapRadioEq(val));
    });
    QObject::connect(&carControlManager, &CarControlManager::radioTrebleChanged,
                     &mediaManager, [&](int val) {
        mediaManager.setEqBand2(mapRadioEq(val));
    });

    SensorManager sensorManager;
    engine.rootContext()->setContextProperty("sensorManager", &sensorManager);

    InternalWiFiManager internalWiFiManager;
    engine.rootContext()->setContextProperty("internalWiFiManager", &internalWiFiManager);

    DebugManager debugManager;
    engine.rootContext()->setContextProperty("debugManager", &debugManager);

    InstallManager installManager;
    engine.rootContext()->setContextProperty("installManager", &installManager);

    DtcManager dtcManager;
    engine.rootContext()->setContextProperty("dtcManager", &dtcManager);

    TripComputer tripComputer(&sensorManager);
    engine.rootContext()->setContextProperty("tripComputer", &tripComputer);

    DataLogger dataLogger(&sensorManager);
    engine.rootContext()->setContextProperty("dataLogger", &dataLogger);

    EngineProfileManager engineProfileManager;
    engine.rootContext()->setContextProperty("engineProfile", &engineProfileManager);
    tripComputer.setEngineProfileManager(&engineProfileManager);

    // Auto-start trip computer when engine is running (RPM > 0)
    QObject::connect(&sensorManager, &SensorManager::rpmChanged,
                     [&tripComputer, &sensorManager]() {
        if (sensorManager.rpm() > 0 && !tripComputer.running())
            tripComputer.start();
        else if (sensorManager.rpm() == 0 && tripComputer.running())
            tripComputer.stop();
    });

    // Auto-start data logger
    dataLogger.startLogging();

    // Link managers to debug manager
    debugManager.setSensorManager(&sensorManager);
    debugManager.setCarControlManager(&carControlManager);
    debugManager.setInternalWiFiManager(&internalWiFiManager);
    debugManager.setMediaManager(&mediaManager);
    debugManager.setCameraManager(&cameraManager);
    debugManager.setAudioManager(&audioManager);

    // ── Auto cooling fan control ────────────────────────────────────────────
    QObject::connect(&sensorManager, &SensorManager::coolantTempChanged,
                     [&sensorManager, &carControlManager, &engineProfileManager]() {
        if (!carControlManager.connected()) return;

        int temp     = sensorManager.coolantTemp();
        int current  = carControlManager.fanRelay();
        int newLevel = current;

        int fanOnHigh  = engineProfileManager.coolantFanOnHighF();
        int fanOffHigh = engineProfileManager.coolantFanOffHighF();
        int fanOnLow   = engineProfileManager.coolantFanOnLowF();
        int fanOffLow  = engineProfileManager.coolantFanOffLowF();

        if      (temp >= fanOnHigh)                newLevel = 2;
        else if (temp >= fanOnLow && current < 1)  newLevel = 1;
        else if (temp < fanOffLow && current >= 1) newLevel = 0;
        else if (temp < fanOffHigh && current == 2) newLevel = 1;

        if (newLevel != current)
            carControlManager.setFanRelay(newLevel);
    });

    // Re-evaluate fan control when profile changes (in case the user switches)
    QObject::connect(&engineProfileManager, &EngineProfileManager::profileChanged,
                     [&sensorManager, &carControlManager]() {
        if (carControlManager.connected())
            emit sensorManager.coolantTempChanged();
    });

    // ── Interior light + chime on door open ──────────────────────────────
    auto onAnyDoorChanged = [&sensorManager, &carControlManager]() {
        if (!carControlManager.connected()) return;
        bool anyOpen = sensorManager.driverDoor() ||
                       sensorManager.passengerDoor() ||
                       sensorManager.trunk() ||
                       sensorManager.hood();
        carControlManager.setExtra(1, anyOpen);
        if (anyOpen)
            carControlManager.playChime();
    };

    QObject::connect(&sensorManager, &SensorManager::driverDoorChanged,    onAnyDoorChanged);
    QObject::connect(&sensorManager, &SensorManager::passengerDoorChanged, onAnyDoorChanged);
    QObject::connect(&sensorManager, &SensorManager::trunkChanged,         onAnyDoorChanged);
    QObject::connect(&sensorManager, &SensorManager::hoodChanged,          onAnyDoorChanged);

    QScreen *screen = app.primaryScreen();
    engine.rootContext()->setContextProperty("screenWidth",  screen->size().width());
    engine.rootContext()->setContextProperty("screenHeight", screen->size().height());
    
    // Expose version to QML
    engine.rootContext()->setContextProperty("appVersion", appVersion);

    const QUrl qmlUrl(QStringLiteral("qrc:/main.qml"));
    engine.load(qmlUrl);

    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
