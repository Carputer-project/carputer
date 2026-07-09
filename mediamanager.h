#ifndef MEDIAMANAGER_H
#define MEDIAMANAGER_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariant>
#include <QDir>
#include <QTimer>
#include <random>
#include <gst/gst.h>
#include <gst/pbutils/pbutils.h>

class ArtworkProvider;

class MediaManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString      currentTrack  READ currentTrack  NOTIFY currentTrackChanged)
    Q_PROPERTY(int          currentIndex  READ currentIndex  NOTIFY currentIndexChanged)
    Q_PROPERTY(int          totalTracks   READ totalTracks   NOTIFY totalTracksChanged)
    Q_PROPERTY(bool         playing       READ playing       NOTIFY playingChanged)
    Q_PROPERTY(qint64       position      READ position      NOTIFY positionChanged)
    Q_PROPERTY(qint64       duration      READ duration      NOTIFY durationChanged)
    Q_PROPERTY(QStringList  playlist      READ playlist      NOTIFY playlistChanged)
    Q_PROPERTY(QVariantList playlistTracks READ playlistTracks NOTIFY playlistChanged)
    Q_PROPERTY(int          volume        READ volume   WRITE setVolume NOTIFY volumeChanged)
    Q_PROPERTY(QString      audioSink     READ audioSink WRITE setAudioSink NOTIFY audioSinkChanged)
    Q_PROPERTY(QVariantList spectrumData  READ spectrumData NOTIFY spectrumDataChanged)
    Q_PROPERTY(QString currentTitle   READ currentTitle   NOTIFY currentTitleChanged)
    Q_PROPERTY(QString currentArtist  READ currentArtist  NOTIFY currentArtistChanged)
    Q_PROPERTY(QString currentAlbum   READ currentAlbum   NOTIFY currentAlbumChanged)
    Q_PROPERTY(QString artworkUrl     READ artworkUrl     NOTIFY artworkUrlChanged)
    Q_PROPERTY(int    repeatMode  READ repeatMode  WRITE setRepeatMode  NOTIFY repeatModeChanged)
    Q_PROPERTY(bool   shuffleOn   READ shuffleOn   WRITE setShuffleOn    NOTIFY shuffleOnChanged)

    // 3-band EQ (GStreamer equalizer-3bands)
    Q_PROPERTY(double eqBand0 READ eqBand0 WRITE setEqBand0 NOTIFY eqBand0Changed)
    Q_PROPERTY(double eqBand1 READ eqBand1 WRITE setEqBand1 NOTIFY eqBand1Changed)
    Q_PROPERTY(double eqBand2 READ eqBand2 WRITE setEqBand2 NOTIFY eqBand2Changed)

public:
    explicit MediaManager(QObject *parent = nullptr);
    ~MediaManager() override;

    QString     currentTrack()  const { return m_currentTrack; }
    int         currentIndex()  const { return m_currentIndex; }
    int         totalTracks()   const { return m_playlist.size(); }
    bool        playing()       const { return m_playing; }
    qint64      position()      const { return m_position; }
    qint64      duration()      const { return m_duration; }
    QStringList playlist()      const { return m_playlist; }
    QVariantList playlistTracks() const { return m_playlistTracks; }
    int         volume()        const { return m_volume; }
    QString     audioSink()     const { return m_audioSink; }
    QVariantList spectrumData() const { return m_spectrumData; }
    QString     currentTitle()  const { return m_currentTitle; }
    QString     currentArtist() const { return m_currentArtist; }
    QString     currentAlbum()  const { return m_currentAlbum; }
    QString     artworkUrl()    const { return m_artworkUrl; }
    int         repeatMode()    const { return m_repeatMode; }
    bool        shuffleOn()     const { return m_shuffleOn; }
    double      eqBand0()       const { return m_eqBand0; }
    double      eqBand1()       const { return m_eqBand1; }
    double      eqBand2()       const { return m_eqBand2; }

    Q_INVOKABLE void play();
    Q_INVOKABLE void pause();
    Q_INVOKABLE void stop();
    Q_INVOKABLE void next();
    Q_INVOKABLE void previous();
    Q_INVOKABLE void seek(qint64 positionMs);
    Q_INVOKABLE void playTrack(int index);
    Q_INVOKABLE void setVolume(int vol);
    Q_INVOKABLE void setAudioSink(const QString &sink);
    Q_INVOKABLE void scanMedia(const QString &path);
    Q_INVOKABLE void addToPlaylist(const QString &path);
    Q_INVOKABLE void clearPlaylist();
    Q_INVOKABLE void setRepeatMode(int mode);
    Q_INVOKABLE void setShuffleOn(bool on);
    Q_INVOKABLE void setEqBand(int band, double gain);
    Q_INVOKABLE void resetEq();
    void setEqBand0(double v);
    void setEqBand1(double v);
    void setEqBand2(double v);

    ArtworkProvider *artworkProvider() const { return m_artworkProvider; }

    // Called from the GStreamer bus watch (not for external use)
    // (now in private slots below)

private slots:
    void positionPollTick();
    void onBusEos();
    void onBusError(const QString &msg);
    void onBusStateChanged(int newState);
    void onSpectrumData(const QVariantList &data);
    void onTagData(const QString &title, const QString &artist,
                   const QString &album, const QByteArray &artwork);

signals:
    void currentTrackChanged();
    void currentIndexChanged();
    void totalTracksChanged();
    void playingChanged();
    void positionChanged();
    void durationChanged();
    void playlistChanged();
    void volumeChanged();
    void audioSinkChanged();
    void errorOccurred(const QString &msg);
    void playbackFinished();
    void spectrumDataChanged();
    void currentTitleChanged();
    void currentArtistChanged();
    void currentAlbumChanged();
    void artworkUrlChanged();
    void repeatModeChanged();
    void shuffleOnChanged();
    void eqBand0Changed();
    void eqBand1Changed();
    void eqBand2Changed();

private:
    void setupPipeline();
    void teardownPipeline();
    void loadAndPlay(int index);
    void updateDuration();
    void scanDirectory(const QString &path);
    void rebuildSink();
    void rebuildPlaylistTracks();
    void generateShuffleOrder();
    int  nextShuffleIndex();
    int  prevShuffleIndex();

    QString      m_currentTrack;
    int          m_currentIndex    = -1;
    bool         m_playing         = false;
    qint64       m_position        = 0;
    qint64       m_duration        = 0;
    QStringList  m_playlist;
    QVariantList m_playlistTracks;
    QStringList  m_audioExtensions;
    QStringList  m_videoExtensions;
    int          m_volume          = 80;
    QString      m_audioSink       = QStringLiteral("default:CARD=MID");
    QVariantList m_spectrumData;
    QString      m_currentTitle;
    QString      m_currentArtist;
    QString      m_currentAlbum;
    QString      m_artworkUrl;

    // 3-band EQ state
    double       m_eqBand0         = 0.0;
    double       m_eqBand1         = 0.0;
    double       m_eqBand2         = 0.0;

    // Repeat: 0=NoRepeat, 1=RepeatAll, 2=RepeatOne
    int          m_repeatMode      = 1;
    bool         m_shuffleOn       = false;
    QList<int>   m_shuffleOrder;
    int          m_shufflePos      = 0;

    GstElement  *m_pipeline        = nullptr;
    GstElement  *m_playbin         = nullptr;
    GstElement  *m_equalizer       = nullptr;
    guint        m_busWatchId       = 0;
    QTimer       m_positionTimer;
    ArtworkProvider *m_artworkProvider = nullptr;

    friend gboolean busCallback(GstBus *bus, GstMessage *msg, gpointer data);
};

#endif // MEDIAMANAGER_H
