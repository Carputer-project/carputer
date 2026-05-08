#ifndef ARTWORKPROVIDER_H
#define ARTWORKPROVIDER_H

#include <QQuickImageProvider>
#include <QImage>
#include <QMutex>

class ArtworkProvider : public QQuickImageProvider
{
public:
    ArtworkProvider()
        : QQuickImageProvider(QQuickImageProvider::Image) {}

    QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize) override;

    void setArtwork(const QImage &img);
    void clear();

private:
    QImage  m_image;
    QMutex  m_mutex;
};

#endif // ARTWORKPROVIDER_H
