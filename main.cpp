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
#include "dvrmanager.h"
#include "carplaymanager.h"
#include "carcontrolmanager.h"
#include "cameramanager.h"
#include "audiomanager.h"
#include "sensormanager.h"
#include "internalwifimanager.h"
#include "debugmanager.h"

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

    DvrManager dvrManager;
    engine.rootContext()->setContextProperty("dvrManager", &dvrManager);

    CarPlayManager carPlayManager;
    engine.rootContext()->setContextProperty("carPlayManager", &carPlayManager);

    CarControlManager carControlManager;
    engine.rootContext()->setContextProperty("carControlManager", &carControlManager);
    // Use configured port if set
    if (!configManager.carControlPort().isEmpty())
        carControlManager.setPort(configManager.carControlPort());

    CameraManager cameraManager;
    engine.rootContext()->setContextProperty("cameraManager", &cameraManager);

    AudioManager audioManager;
    engine.rootContext()->setContextProperty("audioManager", &audioManager);

    SensorManager sensorManager;
    engine.rootContext()->setContextProperty("sensorManager", &sensorManager);

    InternalWiFiManager internalWiFiManager;
    engine.rootContext()->setContextProperty("internalWiFiManager", &internalWiFiManager);

    DebugManager debugManager;
    engine.rootContext()->setContextProperty("debugManager", &debugManager);
    // Link managers to debug manager
    debugManager.setSensorManager(&sensorManager);
    debugManager.setCarControlManager(&carControlManager);
    debugManager.setInternalWiFiManager(&internalWiFiManager);
    debugManager.setMediaManager(&mediaManager);
    debugManager.setCameraManager(&cameraManager);
    debugManager.setAudioManager(&audioManager);

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
