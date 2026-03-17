#pragma once

#include <QObject>
#include <QPixmap>
#include <QHash>

class ImageCache : public QObject {
    Q_OBJECT
public:
    explicit ImageCache(QObject* parent = nullptr);
    ~ImageCache() override;

    QPixmap* get(const QString& url);
    void clear();

signals:
    void imageReady(const QString& url);

private:
    QHash<QString, QPixmap> m_cache;
};
