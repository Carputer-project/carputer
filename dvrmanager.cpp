#include "dvrmanager.h"
#include <QFileInfo>
#include <QDirIterator>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QUrl>
#include <QDateTime>
#include <gst/gst.h>
#include <gst/pbutils/pbutils.h>

// ── GStreamer bus watch (runs on GStreamer thread) ──────────────────────
// Never touch Qt objects directly here — use invokeMethod to cross threads.
gboolean dvrBusCallback(GstBus *bus, GstMessage *msg, gpointer data)
{
    Q_UNUSED(bus);
    DvrManager *self = static_cast<DvrManager*>(data);

    switch (GST_MESSAGE_TYPE(msg)) {

    case GST_MESSAGE_EOS:
        QMetaObject::invokeMethod(self, "onBusEos", Qt::QueuedConnection);
        break;

    case GST_MESSAGE_ERROR: {
        GError *err = nullptr;
        gchar  *dbg = nullptr;
        gst_message_parse_error(msg, &err, &dbg);
        QString errMsg = QString::fromUtf8(err->message);
        g_error_free(err);
        g_free(dbg);
        QMetaObject::invokeMethod(self, "onBusError",
                                   Qt::QueuedConnection,
                                   Q_ARG(QString, errMsg));
        break;
    }

    case GST_MESSAGE_STATE_CHANGED: {
        GstState oldSt, newSt, pending;
        gst_message_parse_state_changed(msg, &oldSt, &newSt, &pending);
        // Only handle state changes from our pipeline
        if (GST_MESSAGE_SRC(msg) == GST_OBJECT(self->m_playPipeline)) {
            QMetaObject::invokeMethod(self, "onBusStateChanged",
                                       Qt::QueuedConnection,
                                       Q_ARG(int, (int)newSt));
        }
        break;
    }

    default:
        break;
    }
    return TRUE;
}

// ── Constructor / Destructor ───────────────────────────────────────────

DvrManager::DvrManager(QObject *parent) : QObject(parent)
{
    gst_init(nullptr, nullptr);

    // Recording process (ffmpeg)
    m_recProcess = new QProcess(this);
    m_recProcess->setProcessChannelMode(QProcess::ForwardedErrorChannel);
    connect(m_recProcess,
            QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &DvrManager::onRecProcessFinished);

    // Recording timer — ticks every second
    m_recTimer.setInterval(1000);
    connect(&m_recTimer, &QTimer::timeout, this, &DvrManager::onRecSecondTick);

    // Playback position timer
    m_positionTimer.setInterval(500);
    connect(&m_positionTimer, &QTimer::timeout,
            this, &DvrManager::positionPollTick);

    ensureDvrDir();
    setupPlaybackPipeline();
    scanRecordings();
}

DvrManager::~DvrManager()
{
    stopRecording();
    teardownPlaybackPipeline();
}

// ── Helpers ────────────────────────────────────────────────────────────

void DvrManager::ensureDvrDir()
{
    QDir d(m_dvrDir);
    if (!d.exists()) d.mkpath(m_dvrDir);
}

QString DvrManager::formatDuration(qint64 ms) const
{
    if (ms <= 0) return QStringLiteral("0:00");
    qint64 s = ms / 1000;
    qint64 m = s / 60;
    s = s % 60;
    qint64 h = m / 60;
    m = m % 60;
    if (h > 0)
        return QString("%1:%2:%3").arg(h).arg(m, 2, 10, QChar('0')).arg(s, 2, 10, QChar('0'));
    return QString("%1:%2").arg(m).arg(s, 2, 10, QChar('0'));
}

QString DvrManager::fileLabel(const QString &path) const
{
    return QFileInfo(path).fileName();
}

// ── Recording ───────────────────────────────────────────────────────────

void DvrManager::startRecording()
{
    if (m_recording) return;

    ensureDvrDir();

    const QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss");
    m_currentFile = QString("%1/dashcam_%2.mkv").arg(m_dvrDir, timestamp);
    emit currentFileChanged();

    QString inputSource = m_cameraSource;
    const QStringList args{
        QStringLiteral("-y"),
        QStringLiteral("-f"),   QStringLiteral("v4l2"),
        QStringLiteral("-i"),   inputSource,
        QStringLiteral("-c:v"), QStringLiteral("mpeg4"),
        QStringLiteral("-q:v"), QStringLiteral("3"),
        QStringLiteral("-an"),
        m_currentFile
    };

    qDebug() << "[DVR] starting ffmpeg:" << args.join(' ');
    m_recProcess->start(QStringLiteral("ffmpeg"), args);

    if (!m_recProcess->waitForStarted(2000)) {
        emit errorOccurred(QStringLiteral("Failed to start ffmpeg — is it installed?"));
        m_currentFile.clear();
        emit currentFileChanged();
        return;
    }

    m_recSeconds = 0;
    m_recording  = true;
    m_recTimer.start();
    emit recordingChanged();
    emit recordingSecondsChanged();
}

void DvrManager::stopRecording()
{
    if (!m_recording) return;

    m_recTimer.stop();
    m_recording = false;
    emit recordingChanged();

    // Send 'q' to ffmpeg stdin for clean file finalisation
    if (m_recProcess->state() == QProcess::Running) {
        m_recProcess->write("q");
        if (!m_recProcess->waitForFinished(3000)) {
            m_recProcess->terminate();
            m_recProcess->waitForFinished(1000);
        }
    }

    scanRecordings();
}

void DvrManager::onRecSecondTick()
{
    m_recSeconds++;
    emit recordingSecondsChanged();
}

void DvrManager::onRecProcessFinished(int code, QProcess::ExitStatus status)
{
    Q_UNUSED(code) Q_UNUSED(status)
    if (m_recording) {
        // Unexpected exit — stop cleanly
        m_recTimer.stop();
        m_recording = false;
        emit recordingChanged();
        emit errorOccurred(QStringLiteral("Recording stopped unexpectedly"));
    }
    scanRecordings();
}

// ── Playback (GStreamer playbin, same as MediaManager) ─────────────────

void DvrManager::setupPlaybackPipeline()
{
    m_playPipeline = gst_pipeline_new("carputer-dvr-playback");
    m_playbin  = gst_element_factory_make("playbin", "dvr-playbin");

    if (!m_playPipeline || !m_playbin) {
        qCritical() << "[DVR] Failed to create GStreamer playback elements";
        return;
    }

    // Audio sink
    GstElement *alsasink = gst_element_factory_make("alsasink", "dvr-alsasink");
    if (alsasink) {
        g_object_set(m_playbin, "audio-sink", alsasink, nullptr);
    }

    gst_bin_add(GST_BIN(m_playPipeline), m_playbin);

    GstBus *bus = gst_pipeline_get_bus(GST_PIPELINE(m_playPipeline));
    m_busWatchId = gst_bus_add_watch(bus, dvrBusCallback, this);
    gst_object_unref(bus);

    qDebug() << "[DVR] Playback pipeline ready";
}

void DvrManager::teardownPlaybackPipeline()
{
    m_positionTimer.stop();
    if (m_playPipeline) {
        if (m_busWatchId > 0) {
            GstBus *bus = gst_pipeline_get_bus(GST_PIPELINE(m_playPipeline));
            g_source_remove(m_busWatchId);
            m_busWatchId = 0;
            gst_object_unref(bus);
        }
        gst_element_set_state(m_playPipeline, GST_STATE_NULL);
        gst_object_unref(m_playPipeline);
        m_playPipeline = nullptr;
        m_playbin  = nullptr;
    }
}

void DvrManager::playFile(const QString &path)
{
    if (!QFile::exists(path)) {
        emit errorOccurred(QString("File not found: %1").arg(path));
        return;
    }

    if (!m_playPipeline || !m_playbin) setupPlaybackPipeline();
    if (!m_playPipeline) return;

    stopPlayback();

    m_playingFile = path;
    emit playingFileChanged();

    QString uri = QUrl::fromLocalFile(path).toString();
    g_object_set(m_playbin, "uri", uri.toUtf8().constData(), nullptr);
    gst_element_set_state(m_playPipeline, GST_STATE_PLAYING);

    qDebug() << "[DVR] Playing:" << path;
}

void DvrManager::stopPlayback()
{
    if (m_playPipeline) {
        gst_element_set_state(m_playPipeline, GST_STATE_NULL);
    }
    m_playing = false;
    m_playPosition = 0;
    m_playDuration = 0;
    emit playingChanged();
    emit playPositionChanged();
    emit playDurationChanged();
}

void DvrManager::togglePause()
{
    if (!m_playPipeline) return;
    GstState cur, pending;
    gst_element_get_state(m_playPipeline, &cur, &pending, 0);
    if (cur == GST_STATE_PLAYING) {
        gst_element_set_state(m_playPipeline, GST_STATE_PAUSED);
    } else if (cur == GST_STATE_PAUSED) {
        gst_element_set_state(m_playPipeline, GST_STATE_PLAYING);
    }
}

void DvrManager::seekTo(qint64 positionMs)
{
    if (!m_playPipeline) return;
    gst_element_seek_simple(m_playPipeline,
                            GST_FORMAT_TIME,
                            (GstSeekFlags)(GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_KEY_UNIT),
                            positionMs * GST_MSECOND);
}

void DvrManager::positionPollTick()
{
    if (!m_playPipeline) return;
    gint64 pos = 0;
    if (gst_element_query_position(m_playPipeline, GST_FORMAT_TIME, &pos)) {
        m_playPosition = pos / GST_MSECOND;
        emit playPositionChanged();
    }
}

void DvrManager::onBusEos()
{
    gst_element_set_state(m_playPipeline, GST_STATE_NULL);
    m_playing  = false;
    m_playPosition = 0;
    emit playingChanged();
    emit playPositionChanged();
}

void DvrManager::onBusError(const QString &msg)
{
    qWarning() << "[DVR] GStreamer error:" << msg;
    gst_element_set_state(m_playPipeline, GST_STATE_NULL);
    m_playing = false;
    emit playingChanged();
    emit errorOccurred(msg);
}

void DvrManager::onBusStateChanged(int newState)
{
    GstState st = (GstState)newState;
    if (st == GST_STATE_PLAYING) {
        if (!m_playing) {
            m_playing = true;
            emit playingChanged();
        }
        // Query duration
        gint64 dur = 0;
        if (gst_element_query_duration(m_playPipeline, GST_FORMAT_TIME, &dur)) {
            m_playDuration = dur / GST_MSECOND;
            emit playDurationChanged();
        }
        m_positionTimer.start();
    } else if (st == GST_STATE_PAUSED) {
        if (m_playing) {
            m_playing = false;
            emit playingChanged();
        }
        m_positionTimer.stop();
    } else if (st == GST_STATE_NULL) {
        m_playing = false;
        m_positionTimer.stop();
        emit playingChanged();
    }
}

// ── File management ────────────────────────────────────────────────────

void DvrManager::scanRecordings()
{
    const QStringList exts{"mkv", "mp4", "avi"};
    QStringList found;

    QDirIterator it(m_dvrDir, QDir::Files | QDir::NoSymLinks);
    while (it.hasNext()) {
        const QString path = it.next();
        if (exts.contains(QFileInfo(path).suffix().toLower()))
            found.prepend(path);
    }
    found.sort();
    std::reverse(found.begin(), found.end()); // newest first

    if (found != m_recordings) {
        m_recordings = found;
        emit recordingsChanged();
    }
}

bool DvrManager::deleteFile(const QString &path)
{
    if (path == m_playingFile && m_playing) {
        emit errorOccurred(QStringLiteral("Stop playback before deleting"));
        return false;
    }
    if (QFile::remove(path)) {
        scanRecordings();
        return true;
    }
    emit errorOccurred(QString("Could not delete: %1").arg(QFileInfo(path).fileName()));
    return false;
}

void DvrManager::setCameraSource(const QString &source)
{
    if (source == m_cameraSource) return;
    m_cameraSource = source;
    emit cameraSourceChanged();
}

void DvrManager::setDvrDirectory(const QString &dir)
{
    if (dir == m_dvrDir) return;
    m_dvrDir = dir;
    ensureDvrDir();
    emit dvrDirectoryChanged();
    scanRecordings();
}
