#include <QApplication>
#include <QImage>
#include <QPainter>
#include "EditorWidget.h"
#include "EditorLayout.h"
#include "EditorPainter.h"
#include "Document.h"
#include <cstdio>

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    
    Document doc;
    doc.insert(0, "Hello World Test");
    
    // 设置选区: 选中 "World"(column 6-11)
    doc.selection().setSelection({0, 6}, {0, 11});
    
    printf("hasSelection: %d\n", doc.selection().hasSelection());
    printf("start: line=%d col=%d\n", doc.selection().range().start().line, doc.selection().range().start().column);
    printf("end: line=%d col=%d\n", doc.selection().range().end().line, doc.selection().range().end().column);
    
    EditorLayout layout;
    QFont font("Consolas", 11);
    font.setStyleHint(QFont::Monospace);
    layout.setFont(font);
    layout.setDocument(&doc);
    layout.rebuild();
    
    printf("lineCount: %d\n", layout.lineCount());
    
    QTextLayout* tl = layout.layoutForLine(0);
    printf("QTextLayout valid: %d\n", tl != nullptr);
    if (tl) {
        printf("QTextLayout lineCount: %d\n", tl->lineCount());
        if (tl->lineCount() > 0) {
            qreal x6 = tl->lineAt(0).cursorToX(6);
            qreal x11 = tl->lineAt(0).cursorToX(11);
            printf("cursorToX(6) = %.2f\n", x6);
            printf("cursorToX(11) = %.2f\n", x11);
            printf("selection width = %.2f\n", x11 - x6);
        }
    }
    
    return 0;
}
