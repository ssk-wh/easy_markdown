#pragma once
#include <QAbstractScrollArea>
#include <QTimer>

class Document;
class EditorLayout;
class EditorPainter;
class EditorInput;
class GutterRenderer;

class EditorWidget : public QAbstractScrollArea {
    Q_OBJECT
public:
    explicit EditorWidget(QWidget* parent = nullptr);
    ~EditorWidget();

    void setDocument(Document* doc);
    Document* document() const;
    EditorLayout* editorLayout() const;

    int gutterWidth() const;

    void ensureCursorVisible();

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void scrollContentsBy(int dx, int dy) override;
    void keyPressEvent(QKeyEvent* event) override;
    void focusInEvent(QFocusEvent* event) override;
    void focusOutEvent(QFocusEvent* event) override;

private slots:
    void onTextChanged(int offset, int removedLen, int addedLen);

private:
    Document* m_doc = nullptr;
    EditorLayout* m_layout = nullptr;
    EditorPainter* m_painter = nullptr;
    EditorInput* m_input = nullptr;
    int m_gutterWidth = 50;

    QTimer m_cursorBlinkTimer;
    bool m_cursorVisible = true;

    void updateScrollBars();
    void updateGutterWidth();
    int firstVisibleLine() const;
    int lastVisibleLine() const;
    qreal scrollY() const;
};
