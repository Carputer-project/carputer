#include "configmanager.h"
#include <QDir>
#include <QDebug>
#include <QStandardPaths>

ConfigManager::ConfigManager(QObject *parent)
    : QObject(parent)
    , m_settings(QSettings::IniFormat, QSettings::UserScope,
                 "carputer", "carputer")
    , m_dvrDirectory(QDir::homePath() + "/dvr_recordings")
{
    // Ensure config directory exists
    QFileInfo info(m_settings.fileName());
    QDir dir;
    if (!dir.exists(info.absolutePath())) {
        dir.mkpath(info.absolutePath());
    }

    reload();
    qDebug() << "[ConfigManager] Settings file:" << m_settings.fileName();
}

// ── Setters (update value + persist immediately) ────────────────────────────

void ConfigManager::setVideoPort(int v)
{
    const int clamped = qBound(1, v, 65535);
    if (m_videoPort == clamped) return;
    m_videoPort = clamped;
    m_settings.setValue("video/port", clamped);
    emit videoPortChanged();
}

void ConfigManager::setVideoCodec(const QString &v)
{
    const QString upper = v.toUpper();
    if (m_videoCodec == upper) return;
    m_videoCodec = upper;
    m_settings.setValue("video/codec", upper);
    emit videoCodecChanged();
}

void ConfigManager::setForceSoftwareDecoding(bool v)
{
    if (m_forceSoftwareDecoding == v) return;
    m_forceSoftwareDecoding = v;
    m_settings.setValue("video/forceSoftwareDecoding", v);
    emit forceSoftwareDecodingChanged();
}

void ConfigManager::setJitterLatencyMs(int v)
{
    const int clamped = qBound(10, v, 500);
    if (m_jitterLatencyMs == clamped) return;
    m_jitterLatencyMs = clamped;
    m_settings.setValue("video/jitterLatencyMs", clamped);
    emit jitterLatencyMsChanged();
}

void ConfigManager::setLocalFallbackPort(int v)
{
    const int clamped = qBound(0, v, 65535);
    if (m_localFallbackPort == clamped) return;
    m_localFallbackPort = clamped;
    m_settings.setValue("video/localFallbackPort", clamped);
    emit localFallbackPortChanged();
}

void ConfigManager::setRtpUrl(const QString &v)
{
    const QString trimmed = v.trimmed();
    if (m_rtpUrl == trimmed) return;
    m_rtpUrl = trimmed;
    m_settings.setValue("video/rtpUrl", trimmed);
    emit rtpUrlChanged();
}

void ConfigManager::setTelemetryPort(int v)
{
    const int clamped = qBound(1, v, 65535);
    if (m_telemetryPort == clamped) return;
    m_telemetryPort = clamped;
    m_settings.setValue("telemetry/port", clamped);
    emit telemetryPortChanged();
}

void ConfigManager::setWfbFrequency(int v)
{
    const int clamped = qBound(2400, v, 5900);  // Valid WiFi frequency range
    if (m_wfbFrequency == clamped) return;
    m_wfbFrequency = clamped;
    m_settings.setValue("wfb/frequency", clamped);
    emit wfbFrequencyChanged();
}

void ConfigManager::setWfbTxPower(int v)
{
    const int clamped = qBound(1, v, 30);  // Typical TX power range in dBm
    if (m_wfbTxPower == clamped) return;
    m_wfbTxPower = clamped;
    m_settings.setValue("wfb/txPower", clamped);
    emit wfbTxPowerChanged();
}

void ConfigManager::setWfbChannelWidth(int v)
{
    // Channel width must be 5, 10, 20, or 40 MHz
    int validWidth = 20;  // default
    if (v == 5 || v == 10 || v == 20 || v == 40) {
        validWidth = v;
    }
    if (m_wfbChannelWidth == validWidth) return;
    m_wfbChannelWidth = validWidth;
    m_settings.setValue("wfb/channelWidth", validWidth);
    emit wfbChannelWidthChanged();
}

void ConfigManager::setWfbLinkId(const QString &v)
{
    if (m_wfbLinkId == v) return;
    m_wfbLinkId = v;
    m_settings.setValue("wfb/linkId", v);
    emit wfbLinkIdChanged();
}

void ConfigManager::setDvrDirectory(const QString &v)
{
    if (m_dvrDirectory == v) return;
    m_dvrDirectory = v;
    m_settings.setValue("dvr/directory", v);
    emit dvrDirectoryChanged();
}

void ConfigManager::setDvrMinFreeMb(int v)
{
    const int clamped = qBound(0, v, 100000);  // Max 100GB
    if (m_dvrMinFreeMb == clamped) return;
    m_dvrMinFreeMb = clamped;
    m_settings.setValue("dvr/minFreeMb", clamped);
    emit dvrMinFreeMbChanged();
}

void ConfigManager::setDvrMaxDuration(int v)
{
    const int clamped = qBound(0, v, 86400);  // Max 24 hours in seconds
    if (m_dvrMaxDuration == clamped) return;
    m_dvrMaxDuration = clamped;
    m_settings.setValue("dvr/maxDurationSec", clamped);
    emit dvrMaxDurationChanged();
}

void ConfigManager::setLastPage(int v)
{
    const int clamped = qBound(1, v, 8);
    if (m_lastPage == clamped) return;
    m_lastPage = clamped;
    m_settings.setValue("ui/lastPage", clamped);
    emit lastPageChanged();
}

void ConfigManager::setVideoFillMode(int v)
{
    const int clamped = qBound(0, v, 2);  // 0=Fit, 1=Crop, 2=Stretch
    if (m_videoFillMode == clamped) return;
    m_videoFillMode = clamped;
    m_settings.setValue("display/videoFillMode", clamped);
    emit videoFillModeChanged();
}

void ConfigManager::setTelemetryVisible(bool v)
{
    if (m_telemetryVisible == v) return;
    m_telemetryVisible = v;
    m_settings.setValue("display/telemetryVisible", v);
    emit telemetryVisibleChanged();
}

void ConfigManager::setHudOpacity(double v)
{
    const double clamped = qBound(0.0, v, 1.0);
    if (qFuzzyCompare(m_hudOpacity, clamped)) return;
    m_hudOpacity = clamped;
    m_settings.setValue("display/hudOpacity", clamped);
    emit hudOpacityChanged();
}

void ConfigManager::setBatteryWarnVolts(double v)
{
    if (qFuzzyCompare(m_batteryWarnVolts, v)) return;
    m_batteryWarnVolts = v;
    m_settings.setValue("display/batteryWarnVolts", v);
    emit batteryWarnVoltsChanged();
}

void ConfigManager::setRemotePort(const QString &v)
{
    const QString trimmed = v.trimmed();
    if (m_remotePort == trimmed) return;
    m_remotePort = trimmed;
    m_settings.setValue("serial/remotePort", trimmed);
    emit remotePortChanged();
}

void ConfigManager::setCarControlPort(const QString &v)
{
    const QString trimmed = v.trimmed();
    if (m_carControlPort == trimmed) return;
    m_carControlPort = trimmed;
    m_settings.setValue("serial/carControlPort", trimmed);
    emit carControlPortChanged();
}

// ── Audio ──────────────────────────────────────────────────────────────

void ConfigManager::setAudioSink(const QString &v)
{
    const QString trimmed = v.trimmed();
    if (m_audioSink == trimmed) return;
    m_audioSink = trimmed.isEmpty() ? QStringLiteral("default") : trimmed;
    m_settings.setValue("audio/sink", m_audioSink);
    emit audioSinkChanged();
}

void ConfigManager::setAudioSource(int v)
{
    const int clamped = qBound(0, v, 2);
    if (m_audioSource == clamped) return;
    m_audioSource = clamped;
    m_settings.setValue("audio/source", clamped);
    emit audioSourceChanged();
}

// ── Climate / HVAC ─────────────────────────────────────────────────────────

void ConfigManager::setTargetTemp(int v)
{
    const int clamped = qBound(60, v, 85);
    if (m_targetTemp == clamped) return;
    m_targetTemp = clamped;
    m_settings.setValue("climate/targetTemp", clamped);
    emit targetTempChanged();
}

void ConfigManager::setFanSpeed(int v)
{
    const int clamped = qBound(0, v, 5);
    if (m_fanSpeed == clamped) return;
    m_fanSpeed = clamped;
    m_settings.setValue("climate/fanSpeed", clamped);
    emit fanSpeedChanged();
}

void ConfigManager::setHvacEnabled(bool v)
{
    if (m_hvacEnabled == v) return;
    m_hvacEnabled = v;
    m_settings.setValue("climate/hvacEnabled", v);
    emit hvacEnabledChanged();
}

void ConfigManager::setAcEnabled(bool v)
{
    if (m_acEnabled == v) return;
    m_acEnabled = v;
    m_settings.setValue("climate/acEnabled", v);
    emit acEnabledChanged();
}

void ConfigManager::setAutoMode(bool v)
{
    if (m_autoMode == v) return;
    m_autoMode = v;
    m_settings.setValue("climate/autoMode", v);
    emit autoModeChanged();
}

void ConfigManager::setRecirculate(bool v)
{
    if (m_recirculate == v) return;
    m_recirculate = v;
    m_settings.setValue("climate/recirculate", v);
    emit recirculateChanged();
}

// ── Theme ─────────────────────────────────────────────────────────────────

void ConfigManager::setTheme(const QString &v)
{
    if (m_theme == v) return;
    m_theme = v;
    m_settings.setValue("ui/theme", v);
    m_settings.sync();
    emit themeChanged();
}

void ConfigManager::setAccentColor(const QString &v)
{
    if (m_accentColor == v) return;
    m_accentColor = v;
    m_settings.setValue("ui/accentColor", v);
    m_settings.sync();
    emit accentColorChanged();
}

// ── Bulk operations ─────────────────────────────────────────────────────────

void ConfigManager::save()
{
    m_settings.setValue("video/port",                 m_videoPort);
    m_settings.setValue("video/codec",                m_videoCodec);
    m_settings.setValue("video/forceSoftwareDecoding", m_forceSoftwareDecoding);
    m_settings.setValue("video/jitterLatencyMs",      m_jitterLatencyMs);
    m_settings.setValue("video/localFallbackPort",    m_localFallbackPort);
    m_settings.setValue("video/rtpUrl",               m_rtpUrl);
    m_settings.setValue("telemetry/port",      m_telemetryPort);
    m_settings.setValue("wfb/frequency",       m_wfbFrequency);
    m_settings.setValue("wfb/txPower",         m_wfbTxPower);
    m_settings.setValue("wfb/channelWidth",    m_wfbChannelWidth);
    m_settings.setValue("wfb/linkId",          m_wfbLinkId);
    m_settings.setValue("dvr/directory",       m_dvrDirectory);
    m_settings.setValue("dvr/minFreeMb",       m_dvrMinFreeMb);
    m_settings.setValue("dvr/maxDurationSec",  m_dvrMaxDuration);
    m_settings.setValue("ui/lastPage",         m_lastPage);
    m_settings.setValue("display/videoFillMode",    m_videoFillMode);
    m_settings.setValue("display/telemetryVisible", m_telemetryVisible);
    m_settings.setValue("display/hudOpacity",       m_hudOpacity);
    m_settings.setValue("display/batteryWarnVolts", m_batteryWarnVolts);
    m_settings.setValue("serial/remotePort",        m_remotePort);
    m_settings.setValue("serial/carControlPort",    m_carControlPort);
    m_settings.setValue("audio/sink",               m_audioSink);
    m_settings.setValue("audio/source",             m_audioSource);
    m_settings.setValue("ui/theme",                m_theme);
    m_settings.setValue("ui/accentColor",          m_accentColor);
    m_settings.sync();
    qDebug() << "[ConfigManager] Settings saved";
}

void ConfigManager::reload()
{
    m_settings.sync();

    m_videoPort       = qBound(1, m_settings.value("video/port", 5600).toInt(), 65535);
    m_videoCodec      = m_settings.value("video/codec",        "H264").toString().toUpper();
    m_forceSoftwareDecoding = m_settings.value("video/forceSoftwareDecoding", false).toBool();
    m_jitterLatencyMs = qBound(10, m_settings.value("video/jitterLatencyMs", 30).toInt(), 500);
    m_localFallbackPort = qBound(0, m_settings.value("video/localFallbackPort", 0).toInt(), 65535);
    m_rtpUrl          = m_settings.value("video/rtpUrl",         "").toString().trimmed();
    m_telemetryPort   = qBound(1, m_settings.value("telemetry/port", 14551).toInt(), 65535);
    m_wfbFrequency    = qBound(2400, m_settings.value("wfb/frequency", 5300).toInt(), 5900);
    m_wfbTxPower      = qBound(1, m_settings.value("wfb/txPower", 20).toInt(), 30);

    // Validate channel width
    int channelWidth  = m_settings.value("wfb/channelWidth", 20).toInt();
    m_wfbChannelWidth = (channelWidth == 5 || channelWidth == 10 || channelWidth == 20 || channelWidth == 40) ? channelWidth : 20;

    m_wfbLinkId       = m_settings.value("wfb/linkId",         DEFAULT_WFB_LINK_ID).toString();
    m_dvrDirectory    = m_settings.value("dvr/directory",
                            QDir::homePath() + "/dvr_recordings").toString();
    m_dvrMinFreeMb    = qBound(0, m_settings.value("dvr/minFreeMb", 500).toInt(), 100000);
    m_dvrMaxDuration  = qBound(0, m_settings.value("dvr/maxDurationSec", 0).toInt(), 86400);
    m_lastPage        = qBound(1, m_settings.value("ui/lastPage", 1).toInt(), 8);
    m_videoFillMode    = qBound(0, m_settings.value("display/videoFillMode", 0).toInt(), 2);
    m_telemetryVisible = m_settings.value("display/telemetryVisible", true).toBool();
    m_hudOpacity       = qBound(0.0, m_settings.value("display/hudOpacity", 1.0).toDouble(), 1.0);
    m_batteryWarnVolts = qBound(0.0, m_settings.value("display/batteryWarnVolts", 10.5).toDouble(), 20.0);
    m_remotePort       = m_settings.value("serial/remotePort", "").toString().trimmed();
    m_carControlPort   = m_settings.value("serial/carControlPort", "").toString().trimmed();
    m_audioSink        = m_settings.value("audio/sink", "default").toString().trimmed();
    if (m_audioSink.isEmpty())
        m_audioSink = QStringLiteral("default");
    m_audioSource      = qBound(0, m_settings.value("audio/source", 0).toInt(), 2);

    // Climate / HVAC
    m_targetTemp    = qBound(60, m_settings.value("climate/targetTemp", 72).toInt(), 85);
    m_fanSpeed      = qBound(0, m_settings.value("climate/fanSpeed", 0).toInt(), 5);
    m_hvacEnabled   = m_settings.value("climate/hvacEnabled", false).toBool();
    m_acEnabled     = m_settings.value("climate/acEnabled", false).toBool();
    m_autoMode      = m_settings.value("climate/autoMode", true).toBool();
    m_recirculate   = m_settings.value("climate/recirculate", false).toBool();

    // Theme
    m_theme = m_settings.value("ui/theme", "Dark").toString();
    m_accentColor = m_settings.value("ui/accentColor", "#00a8e8").toString();

    emit videoPortChanged();
    emit videoCodecChanged();
    emit forceSoftwareDecodingChanged();
    emit jitterLatencyMsChanged();
    emit localFallbackPortChanged();
    emit rtpUrlChanged();
    emit telemetryPortChanged();
    emit wfbFrequencyChanged();
    emit wfbTxPowerChanged();
    emit wfbChannelWidthChanged();
    emit wfbLinkIdChanged();
    emit dvrDirectoryChanged();
    emit dvrMinFreeMbChanged();
    emit dvrMaxDurationChanged();
    emit lastPageChanged();
    emit videoFillModeChanged();
    emit telemetryVisibleChanged();
    emit hudOpacityChanged();
    emit batteryWarnVoltsChanged();
    emit remotePortChanged();
    emit carControlPortChanged();
    emit audioSinkChanged();
    emit audioSourceChanged();

    // Climate / HVAC
    emit targetTempChanged();
    emit fanSpeedChanged();
    emit hvacEnabledChanged();
    emit acEnabledChanged();
    emit autoModeChanged();
    emit recirculateChanged();

    // Theme
    emit themeChanged();
    emit accentColorChanged();

    qDebug() << "[ConfigManager] Settings loaded";
}
