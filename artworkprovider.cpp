#include "artworkprovider.h"

QImage ArtworkProvider::requestImage(const QString &id, QSize *size, const QSize &requestedSize)
{
    Q_UNUSED(id);
    QMutexLocker lock(&m_mutex);

    if (m_image.isNull()) {
        if (size) *size = QSize(0, 0);
        return QImage();
    }

    QImage result = m_image;
    if (requestedSize.isValid() && !requestedSize.isNull())
        result = result.scaled(requestedSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    if (size)
        *size = result.size();

    return result;
}

void ArtworkProvider::setArtwork(const QImage &img)
{
    QMutexLocker lock(&m_mutex);
    m_image = img;
}

void ArtworkProvider::clear()
{
    QMutexLocker lock(&m_mutex);
    m_image = QImage();
}
