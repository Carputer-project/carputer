#include "mediamanager.h"
#include "artworkprovider.h"
#include <QFileInfo>
#include <QDirIterator>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QUrl>
#include <QDateTime>
#include <gst/gst.h>
#include <gst/app/gstappsrc.h>
#include <gst/app/gstappsink.h>
#include <gst/pbutils/pbutils.h>

// ── GStreamer bus watch (runs on GStreamer thread) ─────────────────────────
// Never touch Qt objects directly here — use invokeMethod to cross threads.
gboolean busCallback(GstBus *bus, GstMessage *msg, gpointer data)
{
    Q_UNUSED(bus);
    MediaManager *self = static_cast<MediaManager*>(data);

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

    case GST_MESSAGE_STATE_CHANGED:
        if (GST_MESSAGE_SRC(msg) == GST_OBJECT(self->m_pipeline)) {
            GstState oldSt, newSt, pending;
            gst_message_parse_state_changed(msg, &oldSt, &newSt, &pending);
            QMetaObject::invokeMethod(self, "onBusStateChanged",
                                      Qt::QueuedConnection,
                                      Q_ARG(int, (int)newSt));
        }
        break;

    case GST_MESSAGE_TAG: {
        GstTagList *tags = nullptr;
        gst_message_parse_tag(msg, &tags);
        if (tags) {
            gchar *title  = nullptr;
            gchar *artist = nullptr;
            gchar *album  = nullptr;
            gst_tag_list_get_string(tags, GST_TAG_TITLE,        &title);
            gst_tag_list_get_string(tags, GST_TAG_ARTIST,       &artist);
            gst_tag_list_get_string(tags, GST_TAG_ALBUM,        &album);

            // Extract embedded artwork
            QByteArray artBytes;
            const GValue *imgVal = gst_tag_list_get_value_index(tags, GST_TAG_IMAGE, 0);
            if (!imgVal) imgVal = gst_tag_list_get_value_index(tags, GST_TAG_PREVIEW_IMAGE, 0);
            if (imgVal) {
                GstSample *sample = (GstSample*)g_value_get_boxed(imgVal);
                if (sample) {
                    GstBuffer *buf = gst_sample_get_buffer(sample);
                    if (buf) {
                        GstMapInfo map;
                        if (gst_buffer_map(buf, &map, GST_MAP_READ)) {
                            artBytes = QByteArray((const char*)map.data, (int)map.size);
                            gst_buffer_unmap(buf, &map);
                        }
                    }
                }
            }

            QMetaObject::invokeMethod(self, "onTagData",
                                      Qt::QueuedConnection,
                                      Q_ARG(QString, title  ? QString::fromUtf8(title)  : QString()),
                                      Q_ARG(QString, artist ? QString::fromUtf8(artist) : QString()),
                                      Q_ARG(QString, album  ? QString::fromUtf8(album)  : QString()),
                                      Q_ARG(QByteArray, artBytes));
            g_free(title);
            g_free(artist);
            g_free(album);
            gst_tag_list_unref(tags);
        }
        break;
    }

    case GST_MESSAGE_ELEMENT: {
        const GstStructure *s = gst_message_get_structure(msg);
        if (s && gst_structure_has_name(s, "spectrum")) {
            const GValue *magnitudes = gst_structure_get_value(s, "magnitude");
            if (magnitudes && GST_VALUE_HOLDS_LIST(magnitudes)) {
                QVariantList bands;
                guint size = gst_value_list_get_size(magnitudes);
                bands.reserve(size);
                for (guint i = 0; i < size; i++) {
                    const GValue *v = gst_value_list_get_value(magnitudes, i);
                    bands.append((double)g_value_get_float(v));
                }
                QMetaObject::invokeMethod(self, "onSpectrumData",
                                          Qt::QueuedConnection,
                                          Q_ARG(QVariantList, bands));
            }
        }
        break;
    }

    default:
        break;
    }
    return TRUE;
}

// ── Constructor / Destructor ───────────────────────────────────────────────
MediaManager::MediaManager(QObject *parent)
    : QObject(parent)
{
    qDebug() << "[MediaManager] Constructor start";
    gst_init(nullptr, nullptr);
    m_artworkProvider = new ArtworkProvider();
    qDebug() << "[MediaManager] gst_init done";

    // Only formats confirmed available in the Buildroot image
    m_audioExtensions << "mp3" << "wav" << "m4a" << "aac" << "mp4" << "flac";
    m_videoExtensions << "mp4" << "mkv" << "avi" << "mov" << "webm" << "ts" << "m4v";

    m_positionTimer.setInterval(500);
    connect(&m_positionTimer, &QTimer::timeout,
            this, &MediaManager::positionPollTick);

    // Create common media directories on startup
    QDir().mkpath("/media/usb");
    QDir().mkpath("/media/sd");
    QDir().mkpath("/root/Music");

    qDebug() << "[MediaManager] Calling setupPipeline";
    setupPipeline();
    qDebug() << "[MediaManager] Constructor done";
}

MediaManager::~MediaManager()
{
    teardownPipeline();
    delete m_artworkProvider;
}

// ── Pipeline ───────────────────────────────────────────────────────────────
void MediaManager::setupPipeline()
{
    m_pipeline = gst_pipeline_new("carputer-media");
    m_playbin  = gst_element_factory_make("playbin", "playbin");

    if (!m_pipeline || !m_playbin) {
        qCritical() << "[MediaManager] Failed to create GStreamer elements";
        return;
    }

    // alsasink — device set to m_audioSink ("default" initially)
    GstElement *alsasink = gst_element_factory_make("alsasink", "alsasink");
    if (alsasink) {
        g_object_set(alsasink, "device", m_audioSink.toUtf8().constData(), nullptr);
        g_object_set(m_playbin, "audio-sink", alsasink, nullptr);
    } else {
        qWarning() << "[MediaManager] alsasink unavailable — using playbin default";
    }

    g_object_set(m_playbin, "volume", (double)m_volume / 100.0, nullptr);

    // Audio-filter bin: equalizer → spectrum
    GstElement *audio_filter_bin = gst_bin_new("audio-filter-bin");
    GstElement *last_element = nullptr;

    // Equalizer-3bands (may be null if plugin not in build)
    m_equalizer = gst_element_factory_make("equalizer-3bands", "eq");
    if (m_equalizer) {
        g_object_set(m_equalizer,
                     "band0", m_eqBand0,
                     "band1", m_eqBand1,
                     "band2", m_eqBand2,
                     nullptr);
        gst_bin_add(GST_BIN(audio_filter_bin), m_equalizer);
        last_element = m_equalizer;
        qDebug() << "[MediaManager] equalizer-3bands attached";
    } else {
        qWarning() << "[MediaManager] equalizer-3bands unavailable — no EQ";
    }

    // Spectrum analyzer — 32 bands, -80 dB floor, post every 50 ms
    GstElement *spectrum = gst_element_factory_make("spectrum", "spectrum");
    if (spectrum) {
        g_object_set(spectrum,
                     "bands",         (guint)32,
                     "threshold",     (gint)-80,
                     "post-messages", TRUE,
                     "interval",      (guint64)50000000,
                     nullptr);
        gst_bin_add(GST_BIN(audio_filter_bin), spectrum);
        if (last_element)
            gst_element_link(last_element, spectrum);
        last_element = spectrum;
        qDebug() << "[MediaManager] spectrum analyzer attached";
    } else {
        qWarning() << "[MediaManager] spectrum element unavailable";
    }

    // Ghost pads for the bin
    if (last_element) {
        GstPad *sink_pad = m_equalizer
            ? gst_element_get_static_pad(m_equalizer, "sink")
            : (spectrum ? gst_element_get_static_pad(spectrum, "sink") : nullptr);
        GstPad *src_pad = gst_element_get_static_pad(last_element, "src");

        if (sink_pad) {
            gst_element_add_pad(audio_filter_bin, gst_ghost_pad_new("sink", sink_pad));
            gst_object_unref(sink_pad);
        }
        if (src_pad) {
            gst_element_add_pad(audio_filter_bin, gst_ghost_pad_new("src", src_pad));
            gst_object_unref(src_pad);
        }

        g_object_set(m_playbin, "audio-filter", audio_filter_bin, nullptr);
    } else {
        gst_object_unref(audio_filter_bin);
    }

    gst_bin_add(GST_BIN(m_pipeline), m_playbin);

    GstBus *bus = gst_pipeline_get_bus(GST_PIPELINE(m_pipeline));
    m_busWatchId = gst_bus_add_watch(bus, busCallback, this);
    gst_object_unref(bus);

    qDebug() << "[MediaManager] GStreamer pipeline ready";
}

void MediaManager::teardownPipeline()
{
    m_positionTimer.stop();
    if (m_pipeline) {
        // Remove bus watch first to prevent callbacks during teardown
        if (m_busWatchId > 0) {
            GstBus *bus = gst_pipeline_get_bus(GST_PIPELINE(m_pipeline));
            g_source_remove(m_busWatchId);
            m_busWatchId = 0;
            gst_object_unref(bus);
        }
        gst_element_set_state(m_pipeline, GST_STATE_NULL);
        gst_object_unref(m_pipeline);
        m_pipeline = nullptr;
        m_playbin  = nullptr;
    }
}

void MediaManager::rebuildSink()
{
    // Swap the alsasink to a new device while keeping the pipeline alive
    GstElement *alsasink = gst_element_factory_make("alsasink", "alsasink");
    if (!alsasink) return;
    g_object_set(alsasink, "device", m_audioSink.toUtf8().constData(), nullptr);
    g_object_set(m_playbin, "audio-sink", alsasink, nullptr);
}

// ── Bus handlers (Qt thread) ───────────────────────────────────────────────
void MediaManager::onBusEos()
{
    gst_element_set_state(m_pipeline, GST_STATE_NULL);
    m_playing  = false;
    m_position = 0;
    emit playingChanged();
    emit positionChanged();
    emit playbackFinished();
    // NoRepeat + at end of playlist: don't advance
    if (m_repeatMode == 0 && !m_shuffleOn && m_currentIndex >= m_playlist.size() - 1)
        return;
    if (m_repeatMode == 0 && m_shuffleOn && m_shufflePos >= m_shuffleOrder.size() - 1)
        return;
    QTimer::singleShot(100, this, &MediaManager::next);
}

void MediaManager::onBusError(const QString &msg)
{
    qWarning() << "[MediaManager] GStreamer error:" << msg;
    gst_element_set_state(m_pipeline, GST_STATE_NULL);
    m_playing = false;
    emit playingChanged();
    emit errorOccurred(msg);
    // Don't auto-advance on error — let the user decide
}

void MediaManager::onBusStateChanged(int newState)
{
    GstState st = (GstState)newState;
    if (st == GST_STATE_PLAYING) {
        if (!m_playing) {
            m_playing = true;
            emit playingChanged();
        }
        updateDuration();
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

void MediaManager::onSpectrumData(const QVariantList &data)
{
    m_spectrumData = data;
    emit spectrumDataChanged();
}

void MediaManager::onTagData(const QString &title, const QString &artist,
                              const QString &album, const QByteArray &artwork)
{
    if (!title.isEmpty() && m_currentTitle != title) {
        m_currentTitle = title;
        emit currentTitleChanged();
    }
    if (!artist.isEmpty() && m_currentArtist != artist) {
        m_currentArtist = artist;
        emit currentArtistChanged();
    }
    if (!album.isEmpty() && m_currentAlbum != album) {
        m_currentAlbum = album;
        emit currentAlbumChanged();
    }
    if (!artwork.isEmpty()) {
        // Decode the image using GStreamer (target has no Qt JPEG plugin)
        QImage decoded;
        GstElement *pipe = gst_pipeline_new("artwork-decode");
        GstElement *src  = gst_element_factory_make("appsrc", "src");
        GstElement *dec  = gst_element_factory_make("jpegdec", "dec");
        GstElement *conv = gst_element_factory_make("videoconvert", "conv");
        GstElement *sink = gst_element_factory_make("appsink", "sink");

        if (pipe && src && dec && conv && sink) {
            gst_bin_add_many(GST_BIN(pipe), src, dec, conv, sink, nullptr);
            gst_element_link_many(src, dec, conv, sink, nullptr);

            // Ask for RGB
            GstCaps *caps = gst_caps_new_simple("video/x-raw",
                "format", G_TYPE_STRING, "RGB", nullptr);
            gst_app_sink_set_caps(GST_APP_SINK(sink), caps);
            gst_caps_unref(caps);

            // Push the JPEG data
            GstBuffer *buf = gst_buffer_new_allocate(nullptr, artwork.size(), nullptr);
            gst_buffer_fill(buf, 0, artwork.constData(), artwork.size());
            gst_app_src_push_buffer(GST_APP_SRC(src), buf);
            gst_app_src_end_of_stream(GST_APP_SRC(src));

            gst_element_set_state(pipe, GST_STATE_PLAYING);
            GstSample *sample = gst_app_sink_pull_sample(GST_APP_SINK(sink));
            if (sample) {
                GstBuffer *imgBuf = gst_sample_get_buffer(sample);
                GstCaps *scaps = gst_sample_get_caps(sample);
                GstStructure *s = gst_caps_get_structure(scaps, 0);
                int w = 0, h = 0;
                gst_structure_get_int(s, "width", &w);
                gst_structure_get_int(s, "height", &h);
                if (w > 0 && h > 0) {
                    GstMapInfo map;
                    gst_buffer_map(imgBuf, &map, GST_MAP_READ);
                    // Construct QImage from raw RGB data (no Qt plugin needed)
                    QImage img((const uchar*)map.data, w, h, w * 3, QImage::Format_RGB888);
                    decoded = img.copy(); // deep copy
                    gst_buffer_unmap(imgBuf, &map);
                }
                gst_sample_unref(sample);
            }
            gst_element_set_state(pipe, GST_STATE_NULL);
        } else {
            qWarning() << "[MediaManager] Failed to create artwork decode pipeline";
        }
        if (pipe) gst_object_unref(pipe);

        if (!decoded.isNull()) {
            m_artworkProvider->setArtwork(decoded);
            m_artworkUrl = QStringLiteral("image://artwork/current");
            emit artworkUrlChanged();
        } else {
            qWarning() << "[MediaManager] Failed to decode artwork";
        }
    }
}

// ── Playback controls ──────────────────────────────────────────────────────
void MediaManager::play()
{
    if (!m_pipeline || !m_playbin) return;
    if (m_playlist.isEmpty()) {
        emit errorOccurred(QStringLiteral("Playlist is empty"));
        return;
    }

    if (m_currentIndex < 0) m_currentIndex = 0;

    // If already paused just resume — don't reload the file
    GstState cur, pending;
    gst_element_get_state(m_pipeline, &cur, &pending, 0);
    if (cur == GST_STATE_PAUSED) {
        gst_element_set_state(m_pipeline, GST_STATE_PLAYING);
        return;
    }

    loadAndPlay(m_currentIndex);
}

void MediaManager::pause()
{
    if (!m_pipeline || !m_playing) return;
    gst_element_set_state(m_pipeline, GST_STATE_PAUSED);
}

void MediaManager::stop()
{
    if (!m_pipeline) return;
    m_positionTimer.stop();
    gst_element_set_state(m_pipeline, GST_STATE_NULL);
    m_playing  = false;
    m_position = 0;
    m_duration = 0;
    emit playingChanged();
    emit positionChanged();
    emit durationChanged();
}

void MediaManager::next()
{
    if (m_playlist.isEmpty()) return;
    if (m_repeatMode == 2) {
        // RepeatOne — reload current track
        qint64 savedPos = m_position;
        loadAndPlay(m_currentIndex);
        QTimer::singleShot(300, this, [this, savedPos]() { seek(savedPos); });
        return;
    }
    int idx;
    if (m_shuffleOn) {
        idx = nextShuffleIndex();
    } else {
        idx = m_currentIndex + 1;
        if (idx >= m_playlist.size()) {
            if (m_repeatMode == 0) {
                // NoRepeat — stop at end
                return;
            }
            idx = 0; // RepeatAll wrap
        }
    }
    loadAndPlay(idx);
}

void MediaManager::previous()
{
    if (m_playlist.isEmpty()) return;
    if (m_position > 3000) {
        seek(0);
        return;
    }
    int idx;
    if (m_shuffleOn) {
        idx = prevShuffleIndex();
    } else {
        idx = m_currentIndex - 1;
        if (idx < 0) idx = m_playlist.size() - 1;
    }
    loadAndPlay(idx);
}

void MediaManager::seek(qint64 positionMs)
{
    if (!m_pipeline) return;
    gst_element_seek_simple(m_pipeline,
                            GST_FORMAT_TIME,
                            (GstSeekFlags)(GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_KEY_UNIT),
                            positionMs * GST_MSECOND);
}

void MediaManager::playTrack(int index)
{
    if (index < 0 || index >= m_playlist.size()) return;
    loadAndPlay(index);
}

void MediaManager::setVolume(int vol)
{
    m_volume = qBound(0, vol, 100);
    if (m_playbin)
        g_object_set(m_playbin, "volume", (double)m_volume / 100.0, nullptr);
    emit volumeChanged();
}

void MediaManager::setAudioSink(const QString &sink)
{
    if (m_audioSink == sink) return;
    m_audioSink = sink;
    emit audioSinkChanged();

    if (!m_pipeline) return;

    bool wasPlaying = m_playing;
    qint64 savedPos = m_position;

    gst_element_set_state(m_pipeline, GST_STATE_NULL);
    rebuildSink();

    if (wasPlaying) {
        gst_element_set_state(m_pipeline, GST_STATE_PLAYING);
        QTimer::singleShot(300, this, [this, savedPos]() {
            seek(savedPos);
        });
    }
}

// ── 3-band EQ ──────────────────────────────────────────────────────────────
void MediaManager::setEqBand0(double v)
{
    v = qBound(-24.0, v, 12.0);
    if (qFuzzyCompare(m_eqBand0, v)) return;
    m_eqBand0 = v;
    if (m_equalizer)
        g_object_set(m_equalizer, "band0", v, nullptr);
    emit eqBand0Changed();
}

void MediaManager::setEqBand1(double v)
{
    v = qBound(-24.0, v, 12.0);
    if (qFuzzyCompare(m_eqBand1, v)) return;
    m_eqBand1 = v;
    if (m_equalizer)
        g_object_set(m_equalizer, "band1", v, nullptr);
    emit eqBand1Changed();
}

void MediaManager::setEqBand2(double v)
{
    v = qBound(-24.0, v, 12.0);
    if (qFuzzyCompare(m_eqBand2, v)) return;
    m_eqBand2 = v;
    if (m_equalizer)
        g_object_set(m_equalizer, "band2", v, nullptr);
    emit eqBand2Changed();
}

void MediaManager::setEqBand(int band, double gain)
{
    gain = qBound(-24.0, gain, 12.0);
    switch (band) {
    case 0: setEqBand0(gain); break;
    case 1: setEqBand1(gain); break;
    case 2: setEqBand2(gain); break;
    }
}

void MediaManager::resetEq()
{
    setEqBand0(0.0);
    setEqBand1(0.0);
    setEqBand2(0.0);
}

// ── Internal helpers ───────────────────────────────────────────────────────
void MediaManager::loadAndPlay(int index)
{
    if (!m_pipeline || !m_playbin) return;
    if (index < 0 || index >= m_playlist.size()) return;

    // Stop current playback cleanly
    gst_element_set_state(m_pipeline, GST_STATE_NULL);

    m_currentIndex = index;
    m_currentTrack  = QFileInfo(m_playlist.at(index)).fileName();
    m_currentTitle  = QString();
    m_currentArtist = QString();
    m_currentAlbum  = QString();
    m_artworkUrl    = QString();
    m_position      = 0;
    m_duration      = 0;

    // Check for folder.jpg/cover.jpg in track directory
    QFileInfo fi(m_playlist.at(index));
    QDir trackDir = fi.absoluteDir();
    QStringList coverCandidates = {"folder.jpg", "cover.jpg", "Folder.jpg", "Cover.jpg",
                                   "folder.png", "cover.png", "Folder.png", "Cover.png",
                                   "AlbumArt.jpg", "albumart.jpg"};
    for (const QString &candidate : coverCandidates) {
        QString coverPath = trackDir.absoluteFilePath(candidate);
        if (QFile::exists(coverPath)) {
            m_artworkUrl = QStringLiteral("file://") + coverPath;
            break;
        }
    }

    emit currentIndexChanged();
    emit currentTrackChanged();
    emit currentTitleChanged();
    emit currentArtistChanged();
    emit currentAlbumChanged();
    emit artworkUrlChanged();
    emit positionChanged();
    emit durationChanged();

    QString uri = QUrl::fromLocalFile(m_playlist.at(index)).toString();
    g_object_set(m_playbin, "uri", uri.toUtf8().constData(), nullptr);
    gst_element_set_state(m_pipeline, GST_STATE_PLAYING);

    qDebug() << "[MediaManager] loadAndPlay:" << m_playlist.at(index);
}

void MediaManager::positionPollTick()
{
    if (!m_pipeline) return;
    gint64 pos = 0;
    if (gst_element_query_position(m_pipeline, GST_FORMAT_TIME, &pos)) {
        m_position = pos / GST_MSECOND;
        emit positionChanged();
    }
}

void MediaManager::updateDuration()
{
    if (!m_pipeline) return;
    gint64 dur = 0;
    if (gst_element_query_duration(m_pipeline, GST_FORMAT_TIME, &dur)) {
        m_duration = dur / GST_MSECOND;
        emit durationChanged();
    }
}

// ── Playlist ───────────────────────────────────────────────────────────────
void MediaManager::scanMedia(const QString &path)
{
    m_playlist.clear();
    QDir dir(path);
    if (!dir.exists()) {
        if (dir.mkpath(path)) {
            qDebug() << "[MediaManager] Created directory:" << path;
        } else {
            emit errorOccurred(QString("Could not create directory: %1").arg(path));
            emit playlistChanged();
            emit totalTracksChanged();
            return;
        }
    }
    scanDirectory(path);
    m_playlist.sort();
    rebuildPlaylistTracks();
    if (m_shuffleOn) generateShuffleOrder();
    emit playlistChanged();
    emit totalTracksChanged();
    if (!m_playlist.isEmpty()) {
        m_currentIndex = 0;
        emit currentIndexChanged();
    }
    qDebug() << "[MediaManager] scanned" << m_playlist.size() << "tracks in" << path;
}

void MediaManager::scanDirectory(const QString &path)
{
    QDirIterator it(path, QDir::Files | QDir::NoSymLinks,
                    QDirIterator::Subdirectories);
    while (it.hasNext()) {
        const QString filePath = it.next();
        const QString ext = QFileInfo(filePath).suffix().toLower();
        if (m_audioExtensions.contains(ext) || m_videoExtensions.contains(ext))
            m_playlist.append(filePath);
    }
}

void MediaManager::addToPlaylist(const QString &path)
{
    QFileInfo fi(path);
    if (!fi.isFile()) return;
    const QString ext = fi.suffix().toLower();
    if (m_audioExtensions.contains(ext) || m_videoExtensions.contains(ext)) {
        m_playlist.append(path);
        emit playlistChanged();
        emit totalTracksChanged();
    }
}

void MediaManager::clearPlaylist()
{
    stop();
    m_playlist.clear();
    m_playlistTracks.clear();
    m_currentIndex = -1;
    m_currentTrack.clear();
    emit playlistChanged();
    emit totalTracksChanged();
    emit currentIndexChanged();
    emit currentTrackChanged();
}

void MediaManager::setRepeatMode(int mode)
{
    if (mode < 0 || mode > 2) return;
    if (m_repeatMode == mode) return;
    m_repeatMode = mode;
    emit repeatModeChanged();
}

void MediaManager::setShuffleOn(bool on)
{
    if (m_shuffleOn == on) return;
    m_shuffleOn = on;
    if (m_shuffleOn && !m_playlist.isEmpty())
        generateShuffleOrder();
    emit shuffleOnChanged();
}

void MediaManager::rebuildPlaylistTracks()
{
    m_playlistTracks.clear();
    m_playlistTracks.reserve(m_playlist.size());
    for (const QString &fp : m_playlist) {
        QFileInfo fi(fp);
        QVariantMap m;
        m["filePath"]  = fp;
        m["fileName"]  = fi.fileName();
        m["dirName"]   = fi.absolutePath().section('/', -1);
        m["artist"]    = fi.absolutePath().section('/', -2, -2);
        m_playlistTracks.append(m);
    }
}

void MediaManager::generateShuffleOrder()
{
    m_shuffleOrder.clear();
    m_shuffleOrder.reserve(m_playlist.size());
    for (int i = 0; i < m_playlist.size(); i++)
        m_shuffleOrder.append(i);
    std::shuffle(m_shuffleOrder.begin(), m_shuffleOrder.end(),
                 std::mt19937(std::random_device()()));
    // Start at current index's position in the shuffle order
    m_shufflePos = 0;
    for (int i = 0; i < m_shuffleOrder.size(); i++) {
        if (m_shuffleOrder[i] == m_currentIndex) {
            m_shufflePos = i;
            break;
        }
    }
}

int MediaManager::nextShuffleIndex()
{
    m_shufflePos++;
    if (m_shufflePos >= m_shuffleOrder.size()) {
        if (m_repeatMode == 0) return m_currentIndex; // stay
        // RepeatAll: reshuffle
        m_shufflePos = 0;
        generateShuffleOrder();
    }
    return m_shuffleOrder[m_shufflePos];
}

int MediaManager::prevShuffleIndex()
{
    m_shufflePos--;
    if (m_shufflePos < 0)
        m_shufflePos = m_shuffleOrder.size() - 1;
    return m_shuffleOrder[m_shufflePos];
}
