#pragma once

#include "PreviewLayout.h"
#include "Theme.h"

#include <QPainter>
#include <QVector>
#include <QRectF>
#include <QPair>

struct TextSegment {
    QRectF rect;       // 屏幕坐标（相对于 viewport，已减 scrollY）
    int charStart;     // 在纯文本中的起始字符索引
    int charLen;       // 字符长度
    QString text;      // 段内文本（用于逐字精确定位）
    QFont font;        // 段所用字体
};

// [测试模式] 渲染块信息，用于自动化测试验证
#ifdef ENABLE_TEST_MODE
struct BlockInfo {
    QString type;          // "heading", "paragraph", "code_block", "list_item", "table", "quote", "hr"
    int x, y, width, height;  // 屏幕坐标（已减 scrollY）
    QString content;       // 块内容摘要
    int headingLevel = 0;  // 标题级别（仅标题块）
    int listLevel = 0;     // 列表级别（仅列表块）
};
#endif

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

    // 标记高亮
    void setHighlights(const QVector<QPair<int,int>>& highlights);

    // [测试模式] 获取记录的块信息，并输出到 JSON 文件
#ifdef ENABLE_TEST_MODE
    void saveBlocksToJson(int viewportWidth, int viewportHeight) const;
    const QVector<BlockInfo>& blockInfos() const { return m_blockInfos; }
#endif

private:
    void paintBlock(QPainter* p, const LayoutBlock& block,
                    qreal offsetX, qreal offsetY,
                    qreal scrollY, qreal viewportHeight, qreal viewportWidth);
    void paintInlineRuns(QPainter* p, const LayoutBlock& block,
                         qreal x, qreal y, qreal maxWidth);
    void recordSegment(const QRectF& rect, int charStart, int charLen,
                       const QString& text, const QFont& font);
    void countBlockChars(const LayoutBlock& block);

    Theme m_theme;
    QVector<TextSegment> m_textSegments;
    int m_charCounter = 0;  // 绘制期间的字符计数器
    int m_selStart = -1;
    int m_selEnd = -1;
    QVector<QPair<int,int>> m_highlights;  // 标记高亮范围 (start, end)

#ifdef ENABLE_TEST_MODE
    mutable QVector<BlockInfo> m_blockInfos;  // 记录的块信息（用于测试验证）
#endif
};
