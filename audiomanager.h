#pragma once

#include <QObject>
#include <QString>
#include <QStringList>
#include <QProcess>

class MediaManager;

// ── AudioManager ──────────────────────────────────────────────────────
//
// Owns all audio routing for the carputer.
//
// Local source:
//   GStreamer → ALSA → headphone jack
//
// Volume:
//   setMasterVolume() hits GStreamer pipeline + ALSA
// ─────────────────────────────────────────────────────────────────────

class AudioManager : public QObject
{
    Q_OBJECT

public:
    enum Source { Local = 0, Bluetooth = 1, CarPlay = 2 };
    Q_ENUM(Source)

    Q_PROPERTY(int         source        READ source        NOTIFY sourceChanged)
    Q_PROPERTY(QString     audioSink     READ audioSink     NOTIFY audioSinkChanged)
    Q_PROPERTY(int         masterVolume  READ masterVolume  NOTIFY masterVolumeChanged)
    Q_PROPERTY(QStringList availableSinks READ availableSinks NOTIFY availableSinksChanged)

    explicit AudioManager(QObject *parent = nullptr);
    ~AudioManager() override;

    void setMediaManager(MediaManager *mm);

    Source      source()         const { return m_source; }
    QString     audioSink()      const { return m_audioSink; }
    int         masterVolume()   const { return m_masterVolume; }
    QStringList availableSinks() const { return m_availableSinks; }

    Q_INVOKABLE void setSource(int src);
    Q_INVOKABLE void setAudioSink(const QString &sink);
    Q_INVOKABLE void setMasterVolume(int vol);
    Q_INVOKABLE void refreshSinks();

signals:
    void sourceChanged();
    void audioSinkChanged();
    void masterVolumeChanged();
    void availableSinksChanged();

private slots:
    void onAplayFinished(int exitCode, QProcess::ExitStatus status);

private:
    void applySource();
    void queryAlsaSinks();

    Source      m_source       = Local;
    QString     m_audioSink    = QStringLiteral("default");
    int         m_masterVolume   = 80;
    QStringList m_availableSinks;

    MediaManager *m_media    = nullptr;
    QProcess   *m_aplayProcess = nullptr;
};