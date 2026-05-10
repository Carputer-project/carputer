#pragma once
#include <QObject>
#include <QString>
#include <QStringList>
#include <QProcess>
#include <QTimer>
#include <QDir>
#include <gst/gst.h>
#include <gst/app/gstappsink.h>

class VideoFrameProvider;

class DvrManager : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool    recording        READ recording        NOTIFY recordingChanged)
    Q_PROPERTY(int     recordingSeconds READ recordingSeconds NOTIFY recordingSecondsChanged)
    Q_PROPERTY(QString currentFile      READ currentFile      NOTIFY currentFileChanged)
    Q_PROPERTY(QString cameraSource     READ cameraSource     WRITE setCameraSource  NOTIFY cameraSourceChanged)
    Q_PROPERTY(QString dvrDirectory     READ dvrDirectory     WRITE setDvrDirectory  NOTIFY dvrDirectoryChanged)

    Q_PROPERTY(bool    playing      READ playing      NOTIFY playingChanged)
    Q_PROPERTY(QString playingFile  READ playingFile  NOTIFY playingFileChanged)
    Q_PROPERTY(qint64  playPosition READ playPosition NOTIFY playPositionChanged)
    Q_PROPERTY(qint64  playDuration READ playDuration NOTIFY playDurationChanged)

    Q_PROPERTY(bool    previewActive READ previewActive NOTIFY previewActiveChanged)

    Q_PROPERTY(QStringList recordings READ recordings NOTIFY recordingsChanged)

public:
    explicit DvrManager(QObject *parent = nullptr);
    ~DvrManager() override;

    void setVideoFrameProvider(VideoFrameProvider *provider);

    bool    recording()        const { return m_recording; }
    int     recordingSeconds() const { return m_recSeconds; }
    QString currentFile()      const { return m_currentFile; }
    QString cameraSource()     const { return m_cameraSource; }
    QString dvrDirectory()     const { return m_dvrDir; }

    bool    playing()      const { return m_playing; }
    QString playingFile()  const { return m_playingFile; }
    qint64  playPosition() const { return m_playPosition; }
    qint64  playDuration() const { return m_playDuration; }

    bool    previewActive() const { return m_previewActive; }

    QStringList recordings() const { return m_recordings; }

    Q_INVOKABLE void startRecording();
    Q_INVOKABLE void stopRecording();

    Q_INVOKABLE void playFile(const QString &path);
    Q_INVOKABLE void stopPlayback();
    Q_INVOKABLE void togglePause();
    Q_INVOKABLE void seekTo(qint64 positionMs);

    Q_INVOKABLE void startPreview();
    Q_INVOKABLE void stopPreview();

    Q_INVOKABLE bool deleteFile(const QString &path);
    Q_INVOKABLE void scanRecordings();

    Q_INVOKABLE void setCameraSource(const QString &source);
    Q_INVOKABLE void setDvrDirectory(const QString &dir);

    Q_INVOKABLE QString formatDuration(qint64 ms) const;
    Q_INVOKABLE QString fileLabel(const QString &path) const;

signals:
    void recordingChanged();
    void recordingSecondsChanged();
    void currentFileChanged();
    void cameraSourceChanged();
    void dvrDirectoryChanged();

    void playingChanged();
    void playingFileChanged();
    void playPositionChanged();
    void playDurationChanged();

    void previewActiveChanged();

    void recordingsChanged();
    void errorOccurred(const QString &msg);

private slots:
    void onRecSecondTick();
    void onRecProcessFinished(int code, QProcess::ExitStatus status);
    void positionPollTick();
    void onBusEos();
    void onBusError(const QString &msg);
    void onBusStateChanged(int newState);

private:
    void setupPlaybackPipeline();
    void teardownPlaybackPipeline();
    void setupPreviewPipeline();
    void teardownPreviewPipeline();
    void ensureDvrDir();

    friend gboolean dvrBusCallback(GstBus *bus, GstMessage *msg, gpointer data);
    friend GstFlowReturn onPreviewSample(GstAppSink *appsink, gpointer data);
    friend GstFlowReturn onPlaybackSample(GstAppSink *appsink, gpointer data);

    VideoFrameProvider *m_videoProvider = nullptr;

    // Recording
    QProcess *m_recProcess  = nullptr;
    QTimer    m_recTimer;
    bool      m_recording   = false;
    int       m_recSeconds  = 0;
    QString   m_currentFile;
    QString   m_cameraSource = QStringLiteral("/dev/video0");
    QString   m_dvrDir       = QStringLiteral("/root/dvr");

    // Playback (GStreamer playbin + appsink video)
    GstElement *m_playPipeline = nullptr;
    GstElement *m_playbin     = nullptr;
    GstElement *m_playVideoSink = nullptr;
    guint       m_busWatchId   = 0;
    QTimer      m_positionTimer;
    bool        m_playing        = false;
    QString     m_playingFile;
    qint64      m_playPosition   = 0;
    qint64      m_playDuration   = 0;

    // Preview (v4l2src → appsink)
    GstElement *m_previewPipeline = nullptr;
    GstElement *m_previewAppsink = nullptr;
    guint       m_previewBusWatchId = 0;
    bool        m_previewActive = false;

    // File list
    QStringList m_recordings;
};
