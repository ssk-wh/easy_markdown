#pragma once

#include <QPixmap>

class PreviewBlockCache {
public:
    explicit PreviewBlockCache(int maxBytes = 4 * 1024 * 1024);
    ~PreviewBlockCache();

    QPixmap* get(int sourceStartLine) { return nullptr; }
    void put(int /*sourceStartLine*/, const QPixmap& /*pixmap*/) {}
    void invalidateRange(int /*startLine*/, int /*endLine*/) {}
    void clear() {}

private:
    int m_maxBytes;
};
