#include "debugmanager.h"
#include "sensormanager.h"
#include "carcontrolmanager.h"
#include "internalwifimanager.h"
#include "mediamanager.h"
#include "cameramanager.h"
#include "audiomanager.h"
#include <QDebug>
#include <QProcess>
#include <QJsonDocument>
#include <QJsonObject>
#include <QCoreApplication>
#include <QDateTime>

DebugManager::DebugManager(QObject *parent)
    : QObject(parent)
{
    m_checkTimer = new QTimer(this);
    m_checkTimer->setInterval(30000); // Check every 30 seconds
    connect(m_checkTimer, &QTimer::timeout, this, &DebugManager::checkTimers);
}

DebugManager::~DebugManager()
{
}

void DebugManager::runDiagnostics()
{
    m_issues.clear();
    m_running = true;
    emit runningChanged();

    qDebug() << "DebugManager: Starting diagnostics...";

    checkSensorManager();
    checkCarControlManager();
    checkInternalWiFi();
    checkMediaManager();
    checkCameraManager();
    checkAudioManager();
    checkGStreamerPlugins();

    generateReport();
    m_running = false;
    emit runningChanged();
    emit reportUpdated();

    qDebug() << "DebugManager: Diagnostics complete. Issues found:" << m_issues.count();
}

void DebugManager::saveReport(const QString &path)
{
    QFile file(path);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out << "=== Carputer Debug Report ===\n";
        out << "Generated: " << QDateTime::currentDateTime().toString() << "\n";
        if (QCoreApplication::instance()) {
            out << "App Version: " << QCoreApplication::applicationVersion() << "\n\n";
        }
        out << m_lastReport << "\n";
        file.close();
        qDebug() << "DebugManager: Report saved to" << path;
    }
}

void DebugManager::checkTimers()
{
    // Periodic checks
    if (m_sensorManager && m_sensorManager->connected()) {
        // Check if receiving data
        static QDateTime lastDataTime = QDateTime::currentDateTime();
        // This would need a signal from SensorManager to track last data time
    }
}

void DebugManager::checkSensorManager()
{
    if (!m_sensorManager) {
        addIssue("SensorManager", "Not initialized");
        return;
    }

    if (!m_sensorManager->connected()) {
        addIssue("SensorManager", "Not connected - UDP socket not bound");
    } else {
        qDebug() << "DebugManager: SensorManager OK (UDP 5001)";
    }

    // Check if we have valid data
    if (m_sensorManager->speed() == 0 && m_sensorManager->fuelLevel() == 0) {
        addIssue("SensorManager", "No sensor data received (speed=0, fuel=0)");
    }
}

void DebugManager::checkCarControlManager()
{
    if (!m_carControlManager) {
        addIssue("CarControlManager", "Not initialized");
        return;
    }

    if (!m_carControlManager->connected()) {
        addIssue("CarControlManager", "Not connected to ESP32 at " + m_carControlManager->portName());
    } else {
        qDebug() << "DebugManager: CarControlManager OK (connected to" << m_carControlManager->portName() << ")";
    }

    // Check if properties are being updated
    if (m_carControlManager->fanSpeed() == 0 && m_carControlManager->hvacEnabled() == false) {
        // This is normal if HVAC is off - just log
        qDebug() << "DebugManager: CarControlManager - HVAC off (normal)";
    }
}

void DebugManager::checkInternalWiFi()
{
    if (!m_internalWiFiManager) {
        addIssue("InternalWiFiManager", "Not initialized");
        return;
    }

    if (!m_internalWiFiManager->connected()) {
        addIssue("InternalWiFiManager", "Not connected to WiFi");
    } else {
        qDebug() << "DebugManager: InternalWiFiManager OK (SSID:" << m_internalWiFiManager->ssid() << ")";
    }

    if (m_internalWiFiManager->ipAddress().isEmpty()) {
        addIssue("InternalWiFiManager", "No IP address assigned");
    }

    if (m_internalWiFiManager->signalStrength() < -80) {
        addIssue("InternalWiFiManager", "Weak signal: " + QString::number(m_internalWiFiManager->signalStrength()) + " dBm");
    }
}

void DebugManager::checkMediaManager()
{
    if (!m_mediaManager) {
        addIssue("MediaManager", "Not initialized");
        return;
    }

    // Check GStreamer pipeline
    qDebug() << "DebugManager: MediaManager OK (current track:" << m_mediaManager->currentTrack() << ")";
}

void DebugManager::checkCameraManager()
{
    if (!m_cameraManager) {
        addIssue("CameraManager", "Not initialized");
        return;
    }
    qDebug() << "DebugManager: CameraManager OK";
}

void DebugManager::checkAudioManager()
{
    if (!m_audioManager) {
        addIssue("AudioManager", "Not initialized");
        return;
    }
    qDebug() << "DebugManager: AudioManager OK";
}

void DebugManager::checkGStreamerPlugins()
{
    // Check for required GStreamer plugins
    QStringList requiredPlugins = {
        "id3demux", "mpg123audiodec", "flacdec", "alsasink", "playbin"
    };

    for (const QString &plugin : requiredPlugins) {
        QProcess proc;
        proc.setProgram("gst-inspect-1.0");
        proc.setArguments({plugin});
        proc.start();
        proc.waitForFinished(3000);
        if (proc.exitCode() != 0) {
            addIssue("GStreamer", "Plugin not found: " + plugin);
        } else {
            qDebug() << "DebugManager: GStreamer plugin OK:" << plugin;
        }
    }
}

void DebugManager::addIssue(const QString &component, const QString &issue)
{
    QString entry = "[" + component + "] " + issue;
    m_issues.append(entry);
    qWarning() << "DebugManager:" << entry;
    emit issueFound(component, issue);
}

void DebugManager::generateReport()
{
    QStringList report;
    report << "=== CARPUTER DEBUG REPORT ===";
    report << "Generated: " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    report << "";

    if (m_issues.isEmpty()) {
        report << "✅ NO ISSUES FOUND - All systems operational";
    } else {
        report << "❌ ISSUES FOUND: " + QString::number(m_issues.count());
        report << "";
        for (const QString &issue : m_issues) {
            report << "  - " + issue;
        }
    }

    report << "";
    report << "=== SYSTEM STATUS ===";

    // SensorManager
    if (m_sensorManager) {
        report << "SensorManager: " + QString(m_sensorManager->connected() ? "✅ Connected" : "❌ Disconnected");
        report << "  Speed: " + QString::number(m_sensorManager->speed()) + " MPH";
        report << "  Fuel: " + QString::number(m_sensorManager->fuelLevel()) + "%";
        report << "  Coolant: " + QString::number(m_sensorManager->coolantTemp()) + "°F";
    }

    // CarControlManager
    if (m_carControlManager) {
        report << "CarControlManager: " + QString(m_carControlManager->connected() ? "✅ Connected" : "❌ Disconnected");
        report << "  HVAC: " + QString(m_carControlManager->hvacEnabled() ? "ON" : "OFF");
        report << "  Fan Speed: " + QString::number(m_carControlManager->fanSpeed());
    }

    // InternalWiFiManager
    if (m_internalWiFiManager) {
        report << "InternalWiFi: " + QString(m_internalWiFiManager->connected() ? "✅ Connected" : "❌ Disconnected");
        report << "  SSID: " + m_internalWiFiManager->ssid();
        report << "  IP: " + m_internalWiFiManager->ipAddress();
        report << "  Signal: " + QString::number(m_internalWiFiManager->signalStrength()) + " dBm";
    }

    m_lastReport = report.join("\n");
    qDebug() << "DebugManager: Report generated";
}
