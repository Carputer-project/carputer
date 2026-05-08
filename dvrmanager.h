#pragma once
#include <QObject>
#include <QString>
#include <QStringList>
#include <QProcess>
#include <QTimer>
#include <QDir>
#include <gst/gst.h>

// ── DvrManager ────────────────────────────────────────────────────────
// Two GStreamer pipelines:
//   Recording  — ffmpeg writes V4L2 (or any source) to a timestamped .mkv
//   Playback   — GStreamer playbin (same as MediaManager)
//
// Default DVR directory: /root/dvr/   (created on first start)
// Default camera source: /dev/video0
// ─────────────────────────────────────────────────────────────────────

class DvrManager : public QObject
{
    Q_OBJECT

    // Recording state
    Q_PROPERTY(bool    recording        READ recording        NOTIFY recordingChanged)
    Q_PROPERTY(int     recordingSeconds READ recordingSeconds NOTIFY recordingSecondsChanged)
    Q_PROPERTY(QString currentFile      READ currentFile      NOTIFY currentFileChanged)
    Q_PROPERTY(QString cameraSource     READ cameraSource     WRITE setCameraSource  NOTIFY cameraSourceChanged)
    Q_PROPERTY(QString dvrDirectory     READ dvrDirectory     WRITE setDvrDirectory  NOTIFY dvrDirectoryChanged)

    // Playback state
    Q_PROPERTY(bool    playing      READ playing      NOTIFY playingChanged)
    Q_PROPERTY(QString playingFile  READ playingFile  NOTIFY playingFileChanged)
    Q_PROPERTY(qint64  playPosition READ playPosition NOTIFY playPositionChanged)
    Q_PROPERTY(qint64  playDuration READ playDuration NOTIFY playDurationChanged)

    // File list
    Q_PROPERTY(QStringList recordings READ recordings NOTIFY recordingsChanged)

public:
    explicit DvrManager(QObject *parent = nullptr);
    ~DvrManager() override;

    // Recording
    bool    recording()        const { return m_recording; }
    int     recordingSeconds() const { return m_recSeconds; }
    QString currentFile()      const { return m_currentFile; }
    QString cameraSource()     const { return m_cameraSource; }
    QString dvrDirectory()     const { return m_dvrDir; }

    // Playback
    bool    playing()      const { return m_playing; }
    QString playingFile()  const { return m_playingFile; }
    qint64  playPosition() const { return m_playPosition; }
    qint64  playDuration() const { return m_playDuration; }

    // File list
    QStringList recordings() const { return m_recordings; }

    // Invokable API
    Q_INVOKABLE void startRecording();
    Q_INVOKABLE void stopRecording();

    Q_INVOKABLE void playFile(const QString &path);
    Q_INVOKABLE void stopPlayback();
    Q_INVOKABLE void togglePause();
    Q_INVOKABLE void seekTo(qint64 positionMs);

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
    void ensureDvrDir();

    static gboolean busCallback(GstBus *bus, GstMessage *msg, gpointer data);

    // Recording
    QProcess *m_recProcess  = nullptr;
    QTimer    m_recTimer;
    bool      m_recording   = false;
    int       m_recSeconds  = 0;
    QString   m_currentFile;
    QString   m_cameraSource = QStringLiteral("/dev/video0");
    QString   m_dvrDir       = QStringLiteral("/root/dvr");

    // Playback (GStreamer, same as MediaManager)
    GstElement *m_playPipeline = nullptr;
    GstElement *m_playbin     = nullptr;
    guint       m_busWatchId   = 0;
    QTimer      m_positionTimer;
    bool        m_playing        = false;
    QString     m_playingFile;
    qint64      m_playPosition   = 0;
    qint64      m_playDuration   = 0;

    friend gboolean dvrBusCallback(GstBus *bus, GstMessage *msg, gpointer data);

    // File list
    QStringList m_recordings;
};
