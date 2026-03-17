#include "EditorInput.h"
#include "EditorWidget.h"
#include "EditorLayout.h"
#include "Document.h"

#include <QScrollBar>
#include <algorithm>

EditorInput::EditorInput(EditorWidget* editor)
    : m_editor(editor)
{
}

Document* EditorInput::doc()
{
    return m_editor->document();
}

Selection& EditorInput::sel()
{
    return doc()->selection();
}

EditorLayout* EditorInput::layout()
{
    return m_editor->editorLayout();
}

int EditorInput::posToOffset(TextPosition pos)
{
    return doc()->lineToOffset(pos.line) + pos.column;
}

TextPosition EditorInput::offsetToPos(int offset)
{
    int line = doc()->offsetToLine(offset);
    int col = offset - doc()->lineToOffset(line);
    return {line, col};
}

bool EditorInput::keyPressEvent(QKeyEvent* event)
{
    bool shift = event->modifiers() & Qt::ShiftModifier;
    bool ctrl = event->modifiers() & Qt::ControlModifier;

    switch (event->key()) {
    case Qt::Key_Left:      moveLeft(shift); return true;
    case Qt::Key_Right:     moveRight(shift); return true;
    case Qt::Key_Up:        moveUp(shift); return true;
    case Qt::Key_Down:      moveDown(shift); return true;
    case Qt::Key_Home:
        if (ctrl) moveToDocStart(shift); else moveToLineStart(shift);
        return true;
    case Qt::Key_End:
        if (ctrl) moveToDocEnd(shift); else moveToLineEnd(shift);
        return true;
    case Qt::Key_PageUp:    pageUp(shift); return true;
    case Qt::Key_PageDown:  pageDown(shift); return true;
    case Qt::Key_Backspace: deleteBackward(); return true;
    case Qt::Key_Delete:    deleteForward(); return true;
    case Qt::Key_Return:
    case Qt::Key_Enter:     insertNewLine(); return true;
    case Qt::Key_Tab:
        if (shift) unindent(); else indent();
        return true;
    default: break;
    }

    if (ctrl) {
        switch (event->key()) {
        case Qt::Key_Z: undo(); return true;
        case Qt::Key_Y: redo(); return true;
        case Qt::Key_A: selectAll(); return true;
        default: break;
        }
    }

    // 普通文字输入
    QString text = event->text();
    if (!text.isEmpty() && text[0].isPrint()) {
        insertText(text);
        return true;
    }

    return false;
}

void EditorInput::insertText(const QString& text)
{
    if (sel().hasSelection()) {
        TextPosition start = sel().range().start();
        TextPosition end = sel().range().end();
        int startOff = posToOffset(start);
        int endOff = posToOffset(end);
        doc()->remove(startOff, endOff - startOff);
        sel().setCursorPosition(start);
    }

    int offset = posToOffset(sel().cursorPosition());
    doc()->insert(offset, text);

    TextPosition newPos = offsetToPos(offset + text.length());
    sel().setCursorPosition(newPos);
    sel().resetPreferredColumn();
}

void EditorInput::deleteBackward()
{
    if (sel().hasSelection()) {
        TextPosition start = sel().range().start();
        TextPosition end = sel().range().end();
        int startOff = posToOffset(start);
        int endOff = posToOffset(end);
        doc()->remove(startOff, endOff - startOff);
        sel().setCursorPosition(start);
    } else {
        int offset = posToOffset(sel().cursorPosition());
        if (offset > 0) {
            doc()->remove(offset - 1, 1);
            sel().setCursorPosition(offsetToPos(offset - 1));
        }
    }
    sel().resetPreferredColumn();
}

void EditorInput::deleteForward()
{
    if (sel().hasSelection()) {
        TextPosition start = sel().range().start();
        TextPosition end = sel().range().end();
        int startOff = posToOffset(start);
        int endOff = posToOffset(end);
        doc()->remove(startOff, endOff - startOff);
        sel().setCursorPosition(start);
    } else {
        int offset = posToOffset(sel().cursorPosition());
        if (offset < doc()->length()) {
            doc()->remove(offset, 1);
        }
    }
    sel().resetPreferredColumn();
}

void EditorInput::insertNewLine()
{
    // 自动缩进：获取当前行前导空白
    TextPosition pos = sel().cursorPosition();
    QString currentLine = doc()->lineText(pos.line);
    QString indentStr;
    for (QChar c : currentLine) {
        if (c == ' ' || c == '\t') indentStr += c;
        else break;
    }
    insertText(QStringLiteral("\n") + indentStr);
}

void EditorInput::moveLeft(bool select)
{
    TextPosition pos = sel().cursorPosition();
    if (pos.column > 0) {
        pos.column--;
    } else if (pos.line > 0) {
        pos.line--;
        pos.column = doc()->lineText(pos.line).length();
    }
    moveCursorTo(pos, select);
}

void EditorInput::moveRight(bool select)
{
    TextPosition pos = sel().cursorPosition();
    int lineLen = doc()->lineText(pos.line).length();
    if (pos.column < lineLen) {
        pos.column++;
    } else if (pos.line < doc()->lineCount() - 1) {
        pos.line++;
        pos.column = 0;
    }
    moveCursorTo(pos, select);
}

void EditorInput::moveUp(bool select)
{
    TextPosition pos = sel().cursorPosition();
    if (pos.line > 0) {
        pos.line--;
        int pref = sel().preferredColumn();
        if (pref < 0) pref = pos.column;
        pos.column = qMin(pref, doc()->lineText(pos.line).length());
        sel().setPreferredColumn(pref);
        moveCursorTo(pos, select);
        return;
    }
    pos.column = 0;
    moveCursorTo(pos, select);
}

void EditorInput::moveDown(bool select)
{
    TextPosition pos = sel().cursorPosition();
    if (pos.line < doc()->lineCount() - 1) {
        pos.line++;
        int pref = sel().preferredColumn();
        if (pref < 0) pref = pos.column;
        pos.column = qMin(pref, doc()->lineText(pos.line).length());
        sel().setPreferredColumn(pref);
        moveCursorTo(pos, select);
        return;
    }
    pos.column = doc()->lineText(pos.line).length();
    moveCursorTo(pos, select);
}

void EditorInput::moveToLineStart(bool select)
{
    TextPosition pos = sel().cursorPosition();
    pos.column = 0;
    moveCursorTo(pos, select);
}

void EditorInput::moveToLineEnd(bool select)
{
    TextPosition pos = sel().cursorPosition();
    pos.column = doc()->lineText(pos.line).length();
    moveCursorTo(pos, select);
}

void EditorInput::moveToDocStart(bool select)
{
    moveCursorTo({0, 0}, select);
}

void EditorInput::moveToDocEnd(bool select)
{
    int lastLine = doc()->lineCount() - 1;
    int lastCol = doc()->lineText(lastLine).length();
    moveCursorTo({lastLine, lastCol}, select);
}

void EditorInput::pageUp(bool select)
{
    int linesPerPage = qMax(1, (int)(m_editor->viewport()->height() / layout()->defaultLineHeight()));
    TextPosition pos = sel().cursorPosition();
    pos.line = qMax(0, pos.line - linesPerPage);
    pos.column = qMin(pos.column, doc()->lineText(pos.line).length());
    moveCursorTo(pos, select);
}

void EditorInput::pageDown(bool select)
{
    int linesPerPage = qMax(1, (int)(m_editor->viewport()->height() / layout()->defaultLineHeight()));
    TextPosition pos = sel().cursorPosition();
    pos.line = qMin(doc()->lineCount() - 1, pos.line + linesPerPage);
    pos.column = qMin(pos.column, doc()->lineText(pos.line).length());
    moveCursorTo(pos, select);
}

void EditorInput::undo()
{
    if (doc()->canUndo()) {
        doc()->undo();
    }
}

void EditorInput::redo()
{
    if (doc()->canRedo()) {
        doc()->redo();
    }
}

void EditorInput::selectAll()
{
    TextPosition start = {0, 0};
    int lastLine = doc()->lineCount() - 1;
    int lastCol = doc()->lineText(lastLine).length();
    TextPosition end = {lastLine, lastCol};
    sel().setSelection(start, end);
}

void EditorInput::indent()
{
    insertText(QStringLiteral("    "));
}

void EditorInput::unindent()
{
    TextPosition pos = sel().cursorPosition();
    QString line = doc()->lineText(pos.line);
    int lineStart = doc()->lineToOffset(pos.line);

    // 移除行首最多4个空格
    int removeCount = 0;
    for (int i = 0; i < 4 && i < line.length(); ++i) {
        if (line[i] == ' ') removeCount++;
        else if (line[i] == '\t') { removeCount++; break; }
        else break;
    }

    if (removeCount > 0) {
        doc()->remove(lineStart, removeCount);
        pos.column = qMax(0, pos.column - removeCount);
        sel().setCursorPosition(pos);
        sel().resetPreferredColumn();
    }
}

void EditorInput::moveCursorTo(TextPosition pos, bool select)
{
    if (select) {
        sel().extendSelection(pos);
    } else {
        sel().setCursorPosition(pos);
    }
}
