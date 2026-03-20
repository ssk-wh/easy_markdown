#pragma once

#include <QAbstractScrollArea>
#include <memory>
#include "Theme.h"

class AstNode;
class PreviewLayout;
class PreviewPainter;
class ImageCache;

class PreviewWidget : public QAbstractScrollArea {
    Q_OBJECT
public:
    explicit PreviewWidget(QWidget* parent = nullptr);
    ~PreviewWidget() override;

    PreviewLayout* previewLayout() const;
    void scrollToSourceLine(int line);
    void setTheme(const Theme& theme);
    void setWordWrap(bool enabled);
    bool wordWrap() const { return m_wordWrap; }

public slots:
    void updateAst(std::shared_ptr<AstNode> root);

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void rebuildLayout();
    void scrollContentsBy(int dx, int dy) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void contextMenuEvent(QContextMenuEvent* event) override;

private:
    void updateScrollBars();
    QString extractPlainText() const;
    void extractBlockText(const struct LayoutBlock& block, QString& out) const;
    int textIndexAtPoint(const QPointF& point) const;
    void copySelection();

    Theme m_theme;
    PreviewLayout* m_layout = nullptr;
    PreviewPainter* m_painter = nullptr;
    ImageCache* m_imageCache = nullptr;
    std::shared_ptr<AstNode> m_currentAst;
    bool m_wordWrap = true;

    // 文本选区
    QString m_plainText;
    int m_selStart = -1;
    int m_selEnd = -1;
    bool m_selecting = false;
    qreal m_lastDevicePixelRatio = 0;
};
