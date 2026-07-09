#pragma once
#include <QQuickImageProvider>
#include <QImage>
#include <QMutex>
#include <QHash>
#include <QString>

class VideoFrameProvider : public QQuickImageProvider
{
public:
    VideoFrameProvider()
        : QQuickImageProvider(QQuickImageProvider::Image) {}

    QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize) override;

    void updateFrame(const QString &source, const QImage &frame);
    QImage currentFrame(const QString &source) const;

private:
    QHash<QString, QImage> m_frames;
    mutable QMutex m_mutex;
};
