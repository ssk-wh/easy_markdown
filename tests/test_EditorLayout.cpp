#include <gtest/gtest.h>
#include <QApplication>
#include "EditorLayout.h"
#include "Document.h"

// 需要 QApplication 实例来使用 QTextLayout/QFont
class EditorLayoutTest : public ::testing::Test {
protected:
    static void SetUpTestSuite() {
        if (!QApplication::instance()) {
            static int argc = 1;
            static char* argv[] = { (char*)"test" };
            static QApplication app(argc, argv);
        }
    }

    void SetUp() override {
        doc = new Document();
        layout = new EditorLayout();
        QFont font("Courier New", 12);
        font.setStyleHint(QFont::Monospace);
        layout->setFont(font);
        layout->setDocument(doc);
    }

    void TearDown() override {
        delete layout;
        delete doc;
    }

    Document* doc = nullptr;
    EditorLayout* layout = nullptr;
};

TEST_F(EditorLayoutTest, EmptyDocument) {
    EXPECT_EQ(layout->lineCount(), 1);  // 空文档有 1 行
    EXPECT_GT(layout->defaultLineHeight(), 0);
    EXPECT_DOUBLE_EQ(layout->lineY(0), 0.0);
    EXPECT_GT(layout->totalHeight(), 0);
}

TEST_F(EditorLayoutTest, SingleLine) {
    doc->insert(0, "Hello World");
    layout->rebuild();
    EXPECT_EQ(layout->lineCount(), 1);

    // positionToPoint: 行首
    QPointF p0 = layout->positionToPoint({0, 0});
    EXPECT_DOUBLE_EQ(p0.y(), 0.0);
    EXPECT_NEAR(p0.x(), 0.0, 1.0);

    // positionToPoint: 行中
    QPointF p5 = layout->positionToPoint({0, 5});
    EXPECT_GT(p5.x(), 0);
}

TEST_F(EditorLayoutTest, MultipleLines) {
    doc->insert(0, "Line1\nLine2\nLine3");
    layout->rebuild();
    EXPECT_EQ(layout->lineCount(), 3);

    qreal y0 = layout->lineY(0);
    qreal y1 = layout->lineY(1);
    qreal y2 = layout->lineY(2);

    EXPECT_DOUBLE_EQ(y0, 0.0);
    EXPECT_GT(y1, y0);
    EXPECT_GT(y2, y1);
    EXPECT_NEAR(y1 - y0, layout->defaultLineHeight(), 1.0);
}

TEST_F(EditorLayoutTest, LineAtY) {
    doc->insert(0, "Line1\nLine2\nLine3");
    layout->rebuild();

    EXPECT_EQ(layout->lineAtY(0), 0);
    EXPECT_EQ(layout->lineAtY(layout->lineY(1) + 1), 1);
    EXPECT_EQ(layout->lineAtY(layout->lineY(2) + 1), 2);
    EXPECT_EQ(layout->lineAtY(-10), 0);  // 负值返回0
}

TEST_F(EditorLayoutTest, HitTestBasic) {
    doc->insert(0, "ABCDE");
    layout->rebuild();

    // 点击行首区域
    TextPosition p = layout->hitTest(QPointF(1, 1));
    EXPECT_EQ(p.line, 0);
    EXPECT_EQ(p.column, 0);
}

TEST_F(EditorLayoutTest, CursorRect) {
    doc->insert(0, "Hello");
    layout->rebuild();

    QRectF rect = layout->cursorRect({0, 0});
    EXPECT_GE(rect.width(), 1);
    EXPECT_GT(rect.height(), 0);
}

TEST_F(EditorLayoutTest, UpdateLines) {
    doc->insert(0, "Line1\nLine2");
    layout->rebuild();
    EXPECT_EQ(layout->lineCount(), 2);

    doc->insert(6, "A");  // "Line1\nALine2"
    layout->updateLines(1, 1);  // 只更新第 1 行
    EXPECT_EQ(layout->lineCount(), 2);
}

TEST_F(EditorLayoutTest, RebuildAfterNewLine) {
    doc->insert(0, "AB");
    layout->rebuild();
    EXPECT_EQ(layout->lineCount(), 1);

    doc->insert(1, "\n");  // "A\nB"
    layout->rebuild();  // 行数变化需要 rebuild
    EXPECT_EQ(layout->lineCount(), 2);
}
