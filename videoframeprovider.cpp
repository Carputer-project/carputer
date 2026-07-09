#include "videoframeprovider.h"

QImage VideoFrameProvider::requestImage(const QString &id, QSize *size, const QSize &requestedSize)
{
    QString sourceName = id.section('?', 0, 0);
    QMutexLocker lock(&m_mutex);
    QImage img = m_frames.value(sourceName);
    if (sourceName.startsWith("dvr-playback")) {
        static int reqCount = 0;
        reqCount++;
        if (reqCount % 30 == 0)
            qDebug() << "[VideoProvider] requestImage:" << id << "->" << sourceName
                     << "frame:" << (img.isNull() ? "NULL" : QString("%1x%2").arg(img.width()).arg(img.height()));
    }
    if (img.isNull()) {
        img = QImage(requestedSize.isValid() ? requestedSize : QSize(320, 240),
                     QImage::Format_RGB888);
        img.fill(Qt::black);
    }
    if (size)
        *size = img.size();
    if (requestedSize.isValid() && requestedSize != img.size())
        img = img.scaled(requestedSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    return img;
}

void VideoFrameProvider::updateFrame(const QString &source, const QImage &frame)
{
    QMutexLocker lock(&m_mutex);
    m_frames[source] = frame;
}

QImage VideoFrameProvider::currentFrame(const QString &source) const
{
    QMutexLocker lock(&m_mutex);
    return m_frames.value(source);
}
