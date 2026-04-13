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
    if (isNetworkUrl(url)) {
        return nullptr;
    }

    // Skip already-failed URLs
    if (m_failedUrls.contains(url)) {
        return nullptr;
    }

    // Try local file
    QFileInfo fi(url);
    if (!fi.exists() || !fi.isFile()) {
        m_failedUrls.insert(url);
        return nullptr;
    }

    QPixmap pixmap;
    if (pixmap.load(url)) {
        auto inserted = m_cache.insert(url, pixmap);
        emit imageReady(url);
        return &inserted.value();
    }

    m_failedUrls.insert(url);
    return nullptr;
}

bool ImageCache::isFailed(const QString& url) const
{
    return m_failedUrls.contains(url);
}

bool ImageCache::isNetworkUrl(const QString& url) const
{
    return url.startsWith("http://") || url.startsWith("https://");
}

void ImageCache::clear()
{
    m_cache.clear();
    m_failedUrls.clear();
}
