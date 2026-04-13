#pragma once

#include <QObject>
#include <QPixmap>
#include <QHash>
#include <QSet>

class ImageCache : public QObject {
    Q_OBJECT
public:
    explicit ImageCache(QObject* parent = nullptr);
    ~ImageCache() override;

    QPixmap* get(const QString& url);
    bool isFailed(const QString& url) const;
    bool isNetworkUrl(const QString& url) const;
    void clear();

signals:
    void imageReady(const QString& url);

private:
    QHash<QString, QPixmap> m_cache;
    QSet<QString> m_failedUrls;
};
