#include "dvrmanager.h"
#include "videoframeprovider.h"
#include <QFileInfo>
#include <QDirIterator>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QUrl>
#include <QDateTime>
#include <gst/gst.h>
#include <gst/video/video.h>

static int g_playbackFrameCount = 0;

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
        QMetaObject::invokeMethod(self, "onBusError", Qt::QueuedConnection, Q_ARG(QString, errMsg));
        break;
    }
    case GST_MESSAGE_STATE_CHANGED: {
        GstState oldSt, newSt, pending;
        gst_message_parse_state_changed(msg, &oldSt, &newSt, &pending);
        if (GST_MESSAGE_SRC(msg) == GST_OBJECT(self->m_playPipeline))
            QMetaObject::invokeMethod(self, "onBusStateChanged", Qt::QueuedConnection, Q_ARG(int, (int)newSt));
        break;
    }
    default: break;
    }
    return TRUE;
}

static void extractFrameToProvider(GstElement *appsink, VideoFrameProvider *provider, const QString &sourceName)
{
    if (!provider) return;
    GstSample *sample = gst_app_sink_pull_sample(GST_APP_SINK(appsink));
    if (!sample) {
        qDebug() << "[DVR] extractFrame: no sample";
        return;
    }

    GstCaps *caps = gst_sample_get_caps(sample);
    GstBuffer *buffer = gst_sample_get_buffer(sample);
    if (!caps || !buffer) { gst_sample_unref(sample); qDebug() << "[DVR] extractFrame: no caps/buffer"; return; }

    GstVideoInfo vinfo;
    if (!gst_video_info_from_caps(&vinfo, caps)) { gst_sample_unref(sample); qDebug() << "[DVR] extractFrame: bad caps"; return; }

    GstMapInfo info;
    gst_buffer_map(buffer, &info, GST_MAP_READ);

    int w = GST_VIDEO_INFO_WIDTH(&vinfo);
    int h = GST_VIDEO_INFO_HEIGHT(&vinfo);
    int stride = GST_VIDEO_INFO_PLANE_STRIDE(&vinfo, 0);
    GstVideoFormat fmt = GST_VIDEO_INFO_FORMAT(&vinfo);

    const char *fmtName = gst_video_format_to_string(fmt);
    if (g_playbackFrameCount <= 1)
        qDebug() << "[DVR] extractFrame:" << sourceName << fmtName << w << "x" << h << "stride:" << stride;

    QImage img;
    if (fmt == GST_VIDEO_FORMAT_RGB) {
        img = QImage(info.data, w, h, stride, QImage::Format_RGB888).copy();
    } else if (fmt == GST_VIDEO_FORMAT_BGRx || fmt == GST_VIDEO_FORMAT_BGRA) {
        img = QImage(info.data, w, h, stride, QImage::Format_ARGB32).rgbSwapped().copy();
    } else {
        img = QImage(w, h, QImage::Format_RGB888);
        img.fill(Qt::black);
        qDebug() << "[DVR] extractFrame: unsupported format" << fmtName << "- showing black";
    }

    gst_buffer_unmap(buffer, &info);
    gst_sample_unref(sample);

    if (!img.isNull())
        provider->updateFrame(sourceName, img);
}

GstFlowReturn onPreviewSample(GstAppSink *appsink, gpointer data)
{
    auto *self = static_cast<DvrManager*>(data);
    if (self)
        extractFrameToProvider(GST_ELEMENT(appsink), self->m_videoProvider, QStringLiteral("dvr-live"));
    return GST_FLOW_OK;
}

GstFlowReturn onPlaybackSample(GstAppSink *appsink, gpointer data)
{
    auto *self = static_cast<DvrManager*>(data);
    if (self) {
        g_playbackFrameCount++;
        if (g_playbackFrameCount % 30 == 0)
            qDebug() << "[DVR] Playback frames received:" << g_playbackFrameCount;
        extractFrameToProvider(GST_ELEMENT(appsink), self->m_videoProvider, QStringLiteral("dvr-playback"));
    }
    return GST_FLOW_OK;
}

DvrManager::DvrManager(QObject *parent) : QObject(parent)
{
    gst_init(nullptr, nullptr);

    m_recProcess = new QProcess(this);
    m_recProcess->setProcessChannelMode(QProcess::ForwardedErrorChannel);
    connect(m_recProcess,
            QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &DvrManager::onRecProcessFinished);

    m_recTimer.setInterval(1000);
    connect(&m_recTimer, &QTimer::timeout, this, &DvrManager::onRecSecondTick);

    m_positionTimer.setInterval(500);
    connect(&m_positionTimer, &QTimer::timeout, this, &DvrManager::positionPollTick);

    ensureDvrDir();
    setupPlaybackPipeline();
    scanRecordings();
}

DvrManager::~DvrManager()
{
    stopRecording();
    stopPreview();
    teardownPlaybackPipeline();
}

void DvrManager::setVideoFrameProvider(VideoFrameProvider *provider)
{
    m_videoProvider = provider;
}

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

    const QStringList args{
        QStringLiteral("-y"),
        QStringLiteral("-f"),   QStringLiteral("v4l2"),
        QStringLiteral("-i"),   m_cameraSource,
        QStringLiteral("-c:v"), QStringLiteral("mpeg4"),
        QStringLiteral("-q:v"), QStringLiteral("3"),
        QStringLiteral("-an"),
        m_currentFile
    };

    qDebug() << "[DVR] starting ffmpeg:" << args.join(' ');
    m_recProcess->start(QStringLiteral("ffmpeg"), args);

    if (!m_recProcess->waitForStarted(2000)) {
        emit errorOccurred(QStringLiteral("Failed to start ffmpeg"));
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
        m_recTimer.stop();
        m_recording = false;
        emit recordingChanged();
        emit errorOccurred(QStringLiteral("Recording stopped unexpectedly"));
    }
    scanRecordings();
}

// ── Preview Pipeline (v4l2src → appsink for live camera view) ──────────

void DvrManager::setupPreviewPipeline()
{
    if (m_previewPipeline) return;

    m_previewPipeline = gst_pipeline_new("dvr-preview");
    GstElement *src = gst_element_factory_make("v4l2src", "preview-source");
    GstElement *conv = gst_element_factory_make("videoconvert", "preview-convert");
    m_previewAppsink = gst_element_factory_make("appsink", "preview-appsink");

    if (!src || !conv || !m_previewAppsink) {
        qWarning() << "[DVR] Failed to create preview pipeline elements";
        if (m_previewPipeline) { gst_object_unref(m_previewPipeline); m_previewPipeline = nullptr; }
        return;
    }

    g_object_set(src, "device", m_cameraSource.toUtf8().constData(), nullptr);

    GstCaps *caps = gst_caps_new_simple("video/x-raw",
        "format", G_TYPE_STRING, "RGB", nullptr);
    gst_app_sink_set_caps(GST_APP_SINK(m_previewAppsink), caps);
    gst_caps_unref(caps);

    g_object_set(m_previewAppsink, "sync", FALSE, "drop", TRUE, "max-buffers", 1, nullptr);

    GstAppSinkCallbacks cb;
    memset(&cb, 0, sizeof(cb));
    cb.new_sample = onPreviewSample;
    gst_app_sink_set_callbacks(GST_APP_SINK(m_previewAppsink), &cb, this, nullptr);

    gst_bin_add_many(GST_BIN(m_previewPipeline), src, conv, m_previewAppsink, nullptr);
    gst_element_link_many(src, conv, m_previewAppsink, nullptr);
}

void DvrManager::teardownPreviewPipeline()
{
    if (m_previewPipeline) {
        GstBus *bus = gst_pipeline_get_bus(GST_PIPELINE(m_previewPipeline));
        if (m_previewBusWatchId > 0) {
            g_source_remove(m_previewBusWatchId);
            m_previewBusWatchId = 0;
        }
        if (bus) gst_object_unref(bus);
        gst_element_set_state(m_previewPipeline, GST_STATE_NULL);
        gst_object_unref(m_previewPipeline);
        m_previewPipeline = nullptr;
        m_previewAppsink = nullptr;
    }
}

void DvrManager::startPreview()
{
    if (m_previewActive) return;
    setupPreviewPipeline();
    if (!m_previewPipeline) return;

    m_previewActive = true;
    emit previewActiveChanged();
    gst_element_set_state(m_previewPipeline, GST_STATE_PLAYING);
    qDebug() << "[DVR] Preview started on" << m_cameraSource;
}

void DvrManager::stopPreview()
{
    if (!m_previewActive) return;
    m_previewActive = false;
    emit previewActiveChanged();
    teardownPreviewPipeline();
    qDebug() << "[DVR] Preview stopped";
}

// ── Playback (GStreamer playbin + appsink video) ────────────────────────

void DvrManager::setupPlaybackPipeline()
{
    m_playPipeline = gst_pipeline_new("carputer-dvr-playback");
    m_playbin = gst_element_factory_make("playbin", "dvr-playbin");
    if (!m_playPipeline || !m_playbin) {
        qCritical() << "[DVR] Failed to create GStreamer playback elements";
        return;
    }

    GstElement *alsasink = gst_element_factory_make("alsasink", "dvr-alsasink");
    if (alsasink) {
        g_object_set(alsasink, "device", "default:CARD=MID", nullptr);
        g_object_set(m_playbin, "audio-sink", alsasink, nullptr);
    }

    // Video sink bin: videoconvert ! appsink (converts any format to RGB for us)
    GstElement *videoSinkBin = gst_bin_new("dvr-video-sink-bin");
    GstElement *conv = gst_element_factory_make("videoconvert", "dvr-video-convert");
    m_playVideoSink = gst_element_factory_make("appsink", "dvr-playback-appsink");
    if (videoSinkBin && conv && m_playVideoSink) {
        GstCaps *vcaps = gst_caps_new_simple("video/x-raw",
            "format", G_TYPE_STRING, "RGB", nullptr);
        gst_app_sink_set_caps(GST_APP_SINK(m_playVideoSink), vcaps);
        gst_caps_unref(vcaps);

        g_object_set(m_playVideoSink, "sync", FALSE, "drop", TRUE, "max-buffers", 1, nullptr);

        GstAppSinkCallbacks cb;
        memset(&cb, 0, sizeof(cb));
        cb.new_sample = onPlaybackSample;
        gst_app_sink_set_callbacks(GST_APP_SINK(m_playVideoSink), &cb, this, nullptr);

        gst_bin_add_many(GST_BIN(videoSinkBin), conv, m_playVideoSink, nullptr);
        gst_element_link(conv, m_playVideoSink);

        GstPad *pad = gst_element_get_static_pad(conv, "sink");
        gst_element_add_pad(videoSinkBin, gst_ghost_pad_new("sink", pad));
        gst_object_unref(pad);

        g_object_set(m_playbin, "video-sink", videoSinkBin, nullptr);
    } else {
        if (videoSinkBin) gst_object_unref(videoSinkBin);
    }

    gst_bin_add(GST_BIN(m_playPipeline), m_playbin);

    GstBus *bus = gst_pipeline_get_bus(GST_PIPELINE(m_playPipeline));
    m_busWatchId = gst_bus_add_watch(bus, dvrBusCallback, this);
    gst_object_unref(bus);

    qDebug() << "[DVR] Playback pipeline ready with appsink";
}

void DvrManager::teardownPlaybackPipeline()
{
    m_positionTimer.stop();
    if (m_playPipeline) {
        if (m_busWatchId > 0) {
            GstBus *bus = gst_pipeline_get_bus(GST_PIPELINE(m_playPipeline));
            g_source_remove(m_busWatchId);
            m_busWatchId = 0;
            if (bus) gst_object_unref(bus);
        }
        gst_element_set_state(m_playPipeline, GST_STATE_NULL);
        gst_object_unref(m_playPipeline);
        m_playPipeline = nullptr;
        m_playbin = nullptr;
        m_playVideoSink = nullptr;
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
    g_playbackFrameCount = 0;

    m_playingFile = path;
    emit playingFileChanged();

    QString uri = QUrl::fromLocalFile(path).toString();
    g_object_set(m_playbin, "uri", uri.toUtf8().constData(), nullptr);
    gst_element_set_state(m_playPipeline, GST_STATE_PLAYING);

    qDebug() << "[DVR] Playing:" << path;
}

void DvrManager::stopPlayback()
{
    if (m_playPipeline)
        gst_element_set_state(m_playPipeline, GST_STATE_NULL);
    m_playing = false;
    m_playPosition = 0;
    m_playDuration = 0;
    emit playingChanged();
    emit playPositionChanged();
    emit playDurationChanged();
    // Clear last frame
    if (m_videoProvider)
        m_videoProvider->updateFrame("dvr-playback", QImage());
}

void DvrManager::togglePause()
{
    if (!m_playPipeline) return;
    GstState cur, pending;
    gst_element_get_state(m_playPipeline, &cur, &pending, 0);
    if (cur == GST_STATE_PLAYING)
        gst_element_set_state(m_playPipeline, GST_STATE_PAUSED);
    else if (cur == GST_STATE_PAUSED)
        gst_element_set_state(m_playPipeline, GST_STATE_PLAYING);
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
    m_playing = false;
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
    qDebug() << "[DVR] State changed:" << (st == GST_STATE_VOID_PENDING ? "VOID" : st == GST_STATE_NULL ? "NULL" : st == GST_STATE_READY ? "READY" : st == GST_STATE_PAUSED ? "PAUSED" : st == GST_STATE_PLAYING ? "PLAYING" : "?");
    if (st == GST_STATE_PLAYING) {
        if (!m_playing) {
            m_playing = true;
            emit playingChanged();
        }
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
            found.append(path);
    }
    found.sort();
    std::reverse(found.begin(), found.end());

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
    bool wasPreviewing = m_previewActive;
    if (wasPreviewing) stopPreview();
    m_cameraSource = source;
    emit cameraSourceChanged();
    if (wasPreviewing) startPreview();
}

void DvrManager::setDvrDirectory(const QString &dir)
{
    if (dir == m_dvrDir) return;
    m_dvrDir = dir;
    ensureDvrDir();
    emit dvrDirectoryChanged();
    scanRecordings();
}
