#pragma once

#include <QAbstractScrollArea>
#include <memory>

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

public slots:
    void updateAst(std::shared_ptr<AstNode> root);

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void scrollContentsBy(int dx, int dy) override;

private:
    void updateScrollBars();

    PreviewLayout* m_layout = nullptr;
    PreviewPainter* m_painter = nullptr;
    ImageCache* m_imageCache = nullptr;
    std::shared_ptr<AstNode> m_currentAst;
};
