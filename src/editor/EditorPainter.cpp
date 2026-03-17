#include "EditorPainter.h"
#include "EditorLayout.h"
#include "Document.h"

#include <QPainter>
#include <QTextLayout>

EditorPainter::EditorPainter() = default;

void EditorPainter::paint(QPainter* painter, EditorLayout* layout, Document* doc,
                          int firstLine, int lastLine,
                          int gutterWidth, qreal scrollY,
                          bool cursorVisible,
                          TextPosition cursorPos)
{
    Q_UNUSED(doc);

    // Background
    painter->fillRect(painter->clipBoundingRect(), Qt::white);

    // Text
    painter->setPen(Qt::black);
    painter->setFont(layout->font());

    for (int line = firstLine; line <= lastLine && line < layout->lineCount(); ++line) {
        QTextLayout* tl = layout->layoutForLine(line);
        if (!tl) continue;

        qreal y = layout->lineY(line) - scrollY;
        tl->draw(painter, QPointF(gutterWidth + 8, y));  // 8px left margin
    }

    // 光标绘制
    if (cursorVisible) {
        QRectF cr = layout->cursorRect(cursorPos);
        cr.moveLeft(cr.x() + gutterWidth + 8);  // 加 gutter 和 margin 偏移
        cr.moveTop(cr.y() - scrollY);
        painter->fillRect(cr, Qt::black);
    }
}
