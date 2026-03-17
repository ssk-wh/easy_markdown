#include "ImageCache.h"

#include <QFileInfo>

ImageCache::ImageCache(QObject* parent)
    : QObject(parent)
{
}

ImageCache::~ImageCache() = default;

QPixmap* ImageCache::get(const QString& url)
{
    // Check cache
    auto it = m_cache.find(url);
    if (it != m_cache.end()) {
        return &it.value();
    }

    // Skip network URLs
    if (url.startsWith("http://") || url.startsWith("https://")) {
        return nullptr;
    }

    // Try local file
    QFileInfo fi(url);
    if (!fi.exists() || !fi.isFile()) {
        return nullptr;
    }

    QPixmap pixmap;
    if (pixmap.load(url)) {
        auto inserted = m_cache.insert(url, pixmap);
        emit imageReady(url);
        return &inserted.value();
    }

    return nullptr;
}

void ImageCache::clear()
{
    m_cache.clear();
}
