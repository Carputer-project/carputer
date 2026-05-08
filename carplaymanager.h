#ifndef CARPLAYMANAGER_H
#define CARPLAYMANAGER_H

#include <QObject>
#include <QString>
#include <QByteArray>
#include <QTimer>
#include <QThread>
#include <QMutex>
#include <QImage>

/**
 * CarPlayManager - Manages Apple CarPlay connection and streaming
 *
 * This manager handles:
 * - USB detection and connection to iPhone
 * - H.264 video stream decoding from iPhone
 * - Touch event encoding and sending to iPhone
 * - Audio stream routing (Siri, calls, media)
 * - CarPlay session lifecycle management
 *
 * Based on the OpenAuto/libcarplay protocol approach.
 * Requires external carplay library integration for production use.
 */
class CarPlayManager : public QObject
{
    Q_OBJECT

    // Connection state
    Q_PROPERTY(bool connected READ connected NOTIFY connectedChanged)
    Q_PROPERTY(bool streaming READ streaming NOTIFY streamingChanged)
    Q_PROPERTY(QString deviceName READ deviceName NOTIFY deviceNameChanged)
    Q_PROPERTY(QString connectionStatus READ connectionStatus NOTIFY connectionStatusChanged)

    // Video properties
    Q_PROPERTY(int videoWidth READ videoWidth NOTIFY videoSizeChanged)
    Q_PROPERTY(int videoHeight READ videoHeight NOTIFY videoSizeChanged)
    Q_PROPERTY(int frameRate READ frameRate NOTIFY frameRateChanged)

    // Audio properties
    Q_PROPERTY(bool audioEnabled READ audioEnabled WRITE setAudioEnabled NOTIFY audioEnabledChanged)
    Q_PROPERTY(int audioVolume READ audioVolume WRITE setAudioVolume NOTIFY audioVolumeChanged)
    Q_PROPERTY(bool microphoneEnabled READ microphoneEnabled WRITE setMicrophoneEnabled NOTIFY microphoneEnabledChanged)

    // CarPlay mode
    Q_PROPERTY(bool nightMode READ nightMode WRITE setNightMode NOTIFY nightModeChanged)
    Q_PROPERTY(bool wirelessMode READ wirelessMode NOTIFY wirelessModeChanged)

public:
    // Connection states
    enum ConnectionState {
        Disconnected = 0,
        Detecting,
        Connecting,
        Authenticating,
        Streaming,
        Error
    };
    Q_ENUM(ConnectionState)

    // Touch event types (sent to iPhone)
    enum TouchAction {
        TouchDown = 0,
        TouchMove,
        TouchUp,
        TouchCancel
    };
    Q_ENUM(TouchAction)

    explicit CarPlayManager(QObject *parent = nullptr);
    ~CarPlayManager();

    // Property getters
    bool connected() const { return m_connected; }
    bool streaming() const { return m_streaming; }
    QString deviceName() const { return m_deviceName; }
    QString connectionStatus() const { return m_connectionStatus; }

    int videoWidth() const { return m_videoWidth; }
    int videoHeight() const { return m_videoHeight; }
    int frameRate() const { return m_frameRate; }

    bool audioEnabled() const { return m_audioEnabled; }
    int audioVolume() const { return m_audioVolume; }
    bool microphoneEnabled() const { return m_microphoneEnabled; }

    bool nightMode() const { return m_nightMode; }
    bool wirelessMode() const { return m_wirelessMode; }

    // Connection management
    Q_INVOKABLE void startConnection();
    Q_INVOKABLE void stopConnection();
    Q_INVOKABLE void reconnect();

    // Touch input (called from QML when user touches the CarPlay video surface)
    Q_INVOKABLE void sendTouchEvent(int action, int x, int y, int pointerId = 0);
    Q_INVOKABLE void sendMultiTouch(const QVariantList &touches);

    // Physical button emulation
    Q_INVOKABLE void sendHomeButton();
    Q_INVOKABLE void sendSiriButton();
    Q_INVOKABLE void sendSelectButton();
    Q_INVOKABLE void sendBackButton();
    Q_INVOKABLE void sendMenuButton();
    Q_INVOKABLE void sendPlayPauseButton();
    Q_INVOKABLE void sendNextTrackButton();
    Q_INVOKABLE void sendPreviousTrackButton();

    // Knob/dial input
    Q_INVOKABLE void sendKnobRotation(int delta); // positive = clockwise
    Q_INVOKABLE void sendKnobPress();

    // Audio control
    void setAudioEnabled(bool enabled);
    void setAudioVolume(int volume);
    void setMicrophoneEnabled(bool enabled);

    // Display settings
    void setNightMode(bool enabled);
    Q_INVOKABLE void setVideoResolution(int width, int height);

    // USB device scanning
    Q_INVOKABLE void scanForDevices();
    Q_INVOKABLE QStringList availableDevices() const;

signals:
    // Connection signals
    void connectedChanged();
    void streamingChanged();
    void deviceNameChanged();
    void connectionStatusChanged();
    void connectionStateChanged(ConnectionState state);

    // Video signals
    void videoSizeChanged();
    void frameRateChanged();
    void videoFrameReady(const QByteArray &frameData, int width, int height);
    void videoFrameImageReady(const QImage &frame);

    // Audio signals
    void audioEnabledChanged();
    void audioVolumeChanged();
    void microphoneEnabledChanged();
    void audioDataReady(const QByteArray &audioData);

    // CarPlay mode signals
    void nightModeChanged();
    void wirelessModeChanged();

    // Device detection
    void deviceDetected(const QString &deviceId, const QString &deviceName);
    void deviceRemoved(const QString &deviceId);
    void devicesListChanged();

    // Error handling
    void errorOccurred(const QString &error);
    void warningOccurred(const QString &warning);

private slots:
    void onUsbPollTimeout();
    void onConnectionTimeout();
    void processVideoFrame();
    void processAudioPacket();

private:
    // Internal methods
    void initializeUsb();
    void cleanupUsb();
    void detectIPhoneDevice();
    void establishCarPlaySession();
    void teardownSession();

    void startVideoDecoder();
    void stopVideoDecoder();
    void decodeH264Frame(const QByteArray &nalUnit);

    void startAudioRouter();
    void stopAudioRouter();

    void setConnectionState(ConnectionState state);
    void updateConnectionStatus(const QString &status);

    // Connection state
    bool m_connected;
    bool m_streaming;
    QString m_deviceName;
    QString m_connectionStatus;
    ConnectionState m_connectionState;

    // Video properties
    int m_videoWidth;
    int m_videoHeight;
    int m_frameRate;

    // Audio properties
    bool m_audioEnabled;
    int m_audioVolume;
    bool m_microphoneEnabled;

    // Display mode
    bool m_nightMode;
    bool m_wirelessMode;

    // USB detection
    QTimer m_usbPollTimer;
    QTimer m_connectionTimer;
    QStringList m_detectedDevices;

    // Threading
    QThread *m_videoThread;
    QThread *m_audioThread;
    QMutex m_videoMutex;
    QMutex m_audioMutex;

    // Placeholder for external library integration
    // In production, this would be replaced with actual CarPlay library handles
    void *m_carplayHandle;
    void *m_usbHandle;
};

#endif // CARPLAYMANAGER_H
