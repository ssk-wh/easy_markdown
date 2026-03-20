#pragma once

#include "PreviewLayout.h"
#include "Theme.h"

#include <QPainter>
#include <QVector>
#include <QRectF>

struct TextSegment {
    QRectF rect;       // 屏幕坐标（相对于 viewport，已减 scrollY）
    int charStart;     // 在纯文本中的起始字符索引
    int charLen;       // 字符长度
};

class PreviewPainter {
public:
    PreviewPainter();
    ~PreviewPainter();

    void setTheme(const Theme& theme);
    const Theme& theme() const { return m_theme; }

    void paint(QPainter* painter, const LayoutBlock& root,
               qreal scrollY, qreal viewportHeight, qreal viewportWidth);

    const QVector<TextSegment>& textSegments() const { return m_textSegments; }

    // 选区绘制
    void setSelection(int selStart, int selEnd);

private:
    void paintBlock(QPainter* p, const LayoutBlock& block,
                    qreal offsetX, qreal offsetY,
                    qreal scrollY, qreal viewportHeight, qreal viewportWidth);
    void paintInlineRuns(QPainter* p, const LayoutBlock& block,
                         qreal x, qreal y, qreal maxWidth);
    void recordSegment(const QRectF& rect, int charStart, int charLen);
    void countBlockChars(const LayoutBlock& block);

    Theme m_theme;
    QVector<TextSegment> m_textSegments;
    int m_charCounter = 0;  // 绘制期间的字符计数器
    int m_selStart = -1;
    int m_selEnd = -1;
};
