#pragma once
#include <QtGlobal>
#include <QString>
#include <QVector>
#include <QPair>
#include "Selection.h"
#include "Theme.h"

class QPainter;
class EditorLayout;
class Document;

class EditorPainter {
public:
    EditorPainter();

    void setTheme(const Theme& theme);
    const Theme& theme() const { return m_theme; }
    void setSelectionColor(const QColor& color);

    void paint(QPainter* painter, EditorLayout* layout, Document* doc,
               int firstLine, int lastLine,
               int gutterWidth, qreal scrollY, qreal scrollX = 0,
               bool cursorVisible = false,
               TextPosition cursorPos = {0, 0},
               const QString& preeditString = QString(),
               const QVector<QPair<int,int>>& searchMatches = {},
               int currentMatchIndex = -1);

private:
    Theme m_theme;
    QColor m_selectionColor = QColor(0, 51, 153, 100);
};
