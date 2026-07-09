#pragma once
#include <QObject>
#include <QSettings>

class ConfigManager : public QObject
{
    Q_OBJECT

    // Default values for WFB settings - used in both header and cpp
    static inline const QString DEFAULT_WFB_LINK_ID = QStringLiteral("7669206");

    // ── Video ────────────────────────────────────────────────────────────
    Q_PROPERTY(int    videoPort    READ videoPort    WRITE setVideoPort    NOTIFY videoPortChanged)
    Q_PROPERTY(QString videoCodec  READ videoCodec   WRITE setVideoCodec   NOTIFY videoCodecChanged)
    Q_PROPERTY(bool   forceSoftwareDecoding READ forceSoftwareDecoding
                                            WRITE setForceSoftwareDecoding
                                            NOTIFY forceSoftwareDecodingChanged)
    Q_PROPERTY(int    jitterLatencyMs READ jitterLatencyMs WRITE setJitterLatencyMs
                                                           NOTIFY jitterLatencyMsChanged)
    Q_PROPERTY(int    localFallbackPort READ localFallbackPort WRITE setLocalFallbackPort
                                                               NOTIFY localFallbackPortChanged)
    // RTP URL for remote video stream (e.g., "rtp://192.168.1.100:5600")
    Q_PROPERTY(QString rtpUrl READ rtpUrl WRITE setRtpUrl NOTIFY rtpUrlChanged)

    // ── Telemetry ────────────────────────────────────────────────────────
    Q_PROPERTY(int    telemetryPort READ telemetryPort WRITE setTelemetryPort NOTIFY telemetryPortChanged)

    // ── WFB ──────────────────────────────────────────────────────────────
    Q_PROPERTY(int    wfbFrequency   READ wfbFrequency   WRITE setWfbFrequency   NOTIFY wfbFrequencyChanged)
    Q_PROPERTY(int    wfbTxPower     READ wfbTxPower      WRITE setWfbTxPower     NOTIFY wfbTxPowerChanged)
    Q_PROPERTY(int    wfbChannelWidth READ wfbChannelWidth WRITE setWfbChannelWidth NOTIFY wfbChannelWidthChanged)
    Q_PROPERTY(QString wfbLinkId     READ wfbLinkId       WRITE setWfbLinkId       NOTIFY wfbLinkIdChanged)

    // ── DVR ──────────────────────────────────────────────────────────────
    Q_PROPERTY(QString dvrDirectory   READ dvrDirectory   WRITE setDvrDirectory   NOTIFY dvrDirectoryChanged)
    Q_PROPERTY(int     dvrMinFreeMb   READ dvrMinFreeMb   WRITE setDvrMinFreeMb   NOTIFY dvrMinFreeMbChanged)
    Q_PROPERTY(int     dvrMaxDuration READ dvrMaxDuration  WRITE setDvrMaxDuration NOTIFY dvrMaxDurationChanged)

    // ── UI ───────────────────────────────────────────────────────────────
    Q_PROPERTY(int  lastPage    READ lastPage    WRITE setLastPage    NOTIFY lastPageChanged)

    // ── Theme ────────────────────────────────────────────────────────────
    Q_PROPERTY(QString gaugeBorderMode READ gaugeBorderMode WRITE setGaugeBorderMode NOTIFY gaugeBorderModeChanged)

    // ── Background ───────────────────────────────────────────────────────
    Q_PROPERTY(QString backgroundImage READ backgroundImage WRITE setBackgroundImage NOTIFY backgroundImageChanged)
    Q_PROPERTY(double  backgroundOpacity READ backgroundOpacity WRITE setBackgroundOpacity NOTIFY backgroundOpacityChanged)

    // ── Display / HUD ────────────────────────────────────────────────────
    // 0 = PreserveAspectFit  1 = PreserveAspectCrop  2 = Stretch
    Q_PROPERTY(int    videoFillMode    READ videoFillMode    WRITE setVideoFillMode    NOTIFY videoFillModeChanged)
    Q_PROPERTY(bool   telemetryVisible READ telemetryVisible WRITE setTelemetryVisible NOTIFY telemetryVisibleChanged)
    Q_PROPERTY(double hudOpacity       READ hudOpacity       WRITE setHudOpacity       NOTIFY hudOpacityChanged)
    Q_PROPERTY(double batteryWarnVolts READ batteryWarnVolts WRITE setBatteryWarnVolts NOTIFY batteryWarnVoltsChanged)

    // ── Serial ports ─────────────────────────────────────────────────────
    // Empty string = auto-detect
    Q_PROPERTY(QString remotePort     READ remotePort     WRITE setRemotePort     NOTIFY remotePortChanged)
    Q_PROPERTY(QString carControlPort READ carControlPort WRITE setCarControlPort NOTIFY carControlPortChanged)

    // ── Audio ────────────────────────────────────────────────────────────
    Q_PROPERTY(QString audioSink   READ audioSink   WRITE setAudioSink   NOTIFY audioSinkChanged)
    Q_PROPERTY(int        audioSource READ audioSource WRITE setAudioSource NOTIFY audioSourceChanged)

    // ── Climate / HVAC ─────────────────────────────────────────────────
    Q_PROPERTY(int    targetTemp     READ targetTemp     WRITE setTargetTemp     NOTIFY targetTempChanged)
    Q_PROPERTY(int    fanSpeed       READ fanSpeed       WRITE setFanSpeed       NOTIFY fanSpeedChanged)
    Q_PROPERTY(bool   hvacEnabled    READ hvacEnabled    WRITE setHvacEnabled    NOTIFY hvacEnabledChanged)
    Q_PROPERTY(bool   acEnabled      READ acEnabled      WRITE setAcEnabled      NOTIFY acEnabledChanged)
    Q_PROPERTY(bool   autoMode       READ autoMode       WRITE setAutoMode       NOTIFY autoModeChanged)
    Q_PROPERTY(bool   recirculate    READ recirculate    WRITE setRecirculate    NOTIFY recirculateChanged)

public:
    explicit ConfigManager(QObject *parent = nullptr);

    // Getters
    int     videoPort()             const { return m_videoPort; }
    QString videoCodec()            const { return m_videoCodec; }
    bool    forceSoftwareDecoding() const { return m_forceSoftwareDecoding; }
    int     jitterLatencyMs()       const { return m_jitterLatencyMs; }
    int     localFallbackPort()     const { return m_localFallbackPort; }
    QString rtpUrl()                const { return m_rtpUrl; }
    int     telemetryPort()   const { return m_telemetryPort; }
    int     wfbFrequency()    const { return m_wfbFrequency; }
    int     wfbTxPower()      const { return m_wfbTxPower; }
    int     wfbChannelWidth() const { return m_wfbChannelWidth; }
    QString wfbLinkId()       const { return m_wfbLinkId; }
    QString dvrDirectory()    const { return m_dvrDirectory; }
    int     dvrMinFreeMb()    const { return m_dvrMinFreeMb; }
    int     dvrMaxDuration()  const { return m_dvrMaxDuration; }
    int     lastPage()        const { return m_lastPage; }

    // Display / HUD getters
    int     videoFillMode()    const { return m_videoFillMode; }
    bool    telemetryVisible() const { return m_telemetryVisible; }
    double  hudOpacity()       const { return m_hudOpacity; }
    double  batteryWarnVolts() const { return m_batteryWarnVolts; }

    // Serial port getters
    QString remotePort()     const { return m_remotePort; }
    QString carControlPort() const { return m_carControlPort; }

    // Audio getters
    QString audioSink()   const { return m_audioSink; }
    int     audioSource() const { return m_audioSource; }

    // Climate / HVAC getters
    int    targetTemp()    const { return m_targetTemp; }
    int    fanSpeed()      const { return m_fanSpeed; }
    bool   hvacEnabled()   const { return m_hvacEnabled; }
    bool   acEnabled()     const { return m_acEnabled; }
    bool   autoMode()      const { return m_autoMode; }
    bool   recirculate()   const { return m_recirculate; }

    // Setters
    void setVideoPort(int v);
    void setVideoCodec(const QString &v);
    void setForceSoftwareDecoding(bool v);
    void setJitterLatencyMs(int v);
    void setLocalFallbackPort(int v);
    void setRtpUrl(const QString &v);
    void setTelemetryPort(int v);
    void setWfbFrequency(int v);
    void setWfbTxPower(int v);
    void setWfbChannelWidth(int v);
    void setWfbLinkId(const QString &v);
    void setDvrDirectory(const QString &v);
    void setDvrMinFreeMb(int v);
    void setDvrMaxDuration(int v);
    void setLastPage(int v);

    // Display / HUD setters
    void setVideoFillMode(int v);
    void setTelemetryVisible(bool v);
    void setHudOpacity(double v);
    void setBatteryWarnVolts(double v);

    // Serial port setters
    void setRemotePort(const QString &v);
    void setCarControlPort(const QString &v);

    // Audio setters
    void setAudioSink(const QString &v);
    void setAudioSource(int v);

    // Climate / HVAC setters
    void setTargetTemp(int v);
    void setFanSpeed(int v);
    void setHvacEnabled(bool v);
    void setAcEnabled(bool v);
    void setAutoMode(bool v);
    void setRecirculate(bool v);

    // Theme setters
    Q_INVOKABLE void setTheme(const QString &v);
    Q_INVOKABLE void setAccentColor(const QString &v);
    Q_INVOKABLE void setGaugeBorderMode(const QString &v);
    Q_INVOKABLE void setBackgroundImage(const QString &v);
    void setBackgroundOpacity(double v);
    QString theme() const { return m_theme; }
    QString backgroundImage() const { return m_backgroundImage; }
    double backgroundOpacity() const { return m_backgroundOpacity; }
    QString accentColor() const { return m_accentColor; }
    QString gaugeBorderMode() const { return m_gaugeBorderMode; }

    Q_INVOKABLE void save();
    Q_INVOKABLE void reload();

signals:
    void videoPortChanged();
    void videoCodecChanged();
    void forceSoftwareDecodingChanged();
    void jitterLatencyMsChanged();
    void localFallbackPortChanged();
    void rtpUrlChanged();
    void telemetryPortChanged();
    void wfbFrequencyChanged();
    void wfbTxPowerChanged();
    void wfbChannelWidthChanged();
    void wfbLinkIdChanged();
    void dvrDirectoryChanged();
    void dvrMinFreeMbChanged();
    void dvrMaxDurationChanged();
    void lastPageChanged();
    void themeChanged();
    void accentColorChanged();
    void gaugeBorderModeChanged();
    void backgroundImageChanged();
    void backgroundOpacityChanged();

    // Display / HUD signals
    void videoFillModeChanged();
    void telemetryVisibleChanged();
    void hudOpacityChanged();
    void batteryWarnVoltsChanged();

    // Serial port signals
    void remotePortChanged();
    void carControlPortChanged();

    // Audio signals
    void audioSinkChanged();
    void audioSourceChanged();

    // Climate / HVAC signals
    void targetTempChanged();
    void fanSpeedChanged();
    void hvacEnabledChanged();
    void acEnabledChanged();
    void autoModeChanged();
    void recirculateChanged();

private:
    QSettings m_settings;

    int     m_videoPort       = 5600;
    QString m_videoCodec      = QStringLiteral("H264");
    bool    m_forceSoftwareDecoding = false;
    int     m_jitterLatencyMs = 30;
    int     m_localFallbackPort = 0;  // 0 = disabled
    QString m_rtpUrl;                 // empty = disabled, use local UDP port listening
    int     m_telemetryPort   = 14551;
    int     m_wfbFrequency    = 5300;
    int     m_wfbTxPower      = 20;
    int     m_wfbChannelWidth = 20;
    QString m_wfbLinkId       = DEFAULT_WFB_LINK_ID;
    QString m_dvrDirectory;
    int     m_dvrMinFreeMb    = 500;
    int     m_dvrMaxDuration  = 0;  // 0 = unlimited
    int     m_lastPage        = 1;

    // Display / HUD
    int     m_videoFillMode    = 0;     // 0=Fit, 1=Crop, 2=Stretch
    bool    m_telemetryVisible = true;
    double  m_hudOpacity       = 1.0;
    double  m_batteryWarnVolts = 10.5;

    // Serial ports (empty = auto-detect)
    QString m_remotePort;       // RemoteManager port (default: first ttyUSB)
    QString m_carControlPort;   // CarControlManager port (default: second ttyUSB)

    // Audio
    QString m_audioSink = QStringLiteral("default");
    int     m_audioSource = 0;  // 0=Local, 1=Bluetooth, 2=CarPlay

    // Climate / HVAC
    int     m_targetTemp    = 72;
    int     m_fanSpeed      = 0;
    bool    m_hvacEnabled   = false;
    bool    m_acEnabled     = false;
    bool    m_autoMode      = true;
    bool    m_recirculate   = false;

    // Theme
    QString m_theme = QStringLiteral("Dark");
    QString m_accentColor = QStringLiteral("#00a8e8");
    QString m_gaugeBorderMode = QStringLiteral("gauge");
    QString m_backgroundImage;
    double m_backgroundOpacity = 0.3;
};
