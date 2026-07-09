#include "audiomanager.h"
#include "mediamanager.h"

#include <QProcess>
#include <QDebug>

AudioManager::AudioManager(QObject *parent)
    : QObject(parent)
    , m_aplayProcess(new QProcess(this))
{
    queryAlsaSinks();

    connect(m_aplayProcess,
            QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &AudioManager::onAplayFinished);
}

AudioManager::~AudioManager()
{
    if (m_aplayProcess->state() != QProcess::NotRunning) {
        m_aplayProcess->kill();
        m_aplayProcess->waitForFinished(1000);
    }
}

void AudioManager::setMediaManager(MediaManager *mm) { m_media = mm; }

void AudioManager::setSource(int src)
{
    const Source s = static_cast<Source>(src);
    qDebug() << "[AudioManager] setSource(" << src << ") current=" << m_source;
    if (m_source == s) return;
    m_source = s;
    applySource();
    emit sourceChanged();
}

void AudioManager::applySource()
{
    switch (m_source) {
    case Local:
        qDebug() << "[AudioManager] → Local";
        if (m_media) m_media->play();
        break;
    case Bluetooth:
        qDebug() << "[AudioManager] → Bluetooth";
        if (m_media && m_media->playing()) m_media->pause();
        break;
    case CarPlay:
        qDebug() << "[AudioManager] → CarPlay";
        if (m_media && m_media->playing()) m_media->pause();
        break;
    }
}

void AudioManager::setAudioSink(const QString &sink)
{
    if (m_audioSink == sink) return;
    m_audioSink = sink;
    if (m_media) m_media->setAudioSink(sink);
    emit audioSinkChanged();
    qDebug() << "[AudioManager] audio sink →" << sink;
}

void AudioManager::setMasterVolume(int vol)
{
    m_masterVolume = qBound(0, vol, 100);
    if (m_media) m_media->setVolume(m_masterVolume);
    emit masterVolumeChanged();
}

void AudioManager::refreshSinks() { queryAlsaSinks(); }

void AudioManager::onAplayFinished(int exitCode, QProcess::ExitStatus)
{
    qDebug() << "[AudioManager] aplay exited with code" << exitCode;
}

void AudioManager::queryAlsaSinks()
{
    QProcess p;
    p.start(QStringLiteral("aplay"), QStringList{ QStringLiteral("-L") });
    p.waitForFinished(3000);

    QStringList sinks;
    sinks << QStringLiteral("default");
    const QString out = QString::fromLocal8Bit(p.readAllStandardOutput());
    for (const QString &line : out.split(QLatin1Char('\n'))) {
        const QString dev = line.trimmed();
        if (dev.isEmpty()) continue;
        if (dev == QLatin1String("default")) continue;
        if (dev.startsWith(QLatin1String("null"))) continue;
        if (dev.startsWith(QLatin1String("pulse"))) continue;
        sinks << dev;
    }
    if (m_availableSinks != sinks) {
        m_availableSinks = sinks;
        emit availableSinksChanged();
        qDebug() << "[AudioManager] sinks:" << sinks;
    }
}
