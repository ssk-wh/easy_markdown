#pragma once

#include <QWidget>
#include <QVector>
#include <QString>
#include <QSet>
#include "Theme.h"

class QScrollArea;
class QVBoxLayout;

struct TocEntry {
    QString title;
    int level = 1;
    int sourceLine = 0;
};

class TocPanel : public QWidget {
    Q_OBJECT
public:
    explicit TocPanel(QWidget* parent = nullptr);

    void setEntries(const QVector<TocEntry>& entries);
    void setTheme(const Theme& theme);
    void setHighlightedEntries(const QSet<int>& indices);

    const QVector<TocEntry>& entries() const { return m_entries; }
    const QSet<int>& highlightedEntries() const { return m_highlightedEntries; }

signals:
    void headingClicked(int sourceLine);

private:
    void buildList();

    QScrollArea* m_scrollArea;
    QVBoxLayout* m_listLayout;
    QVector<TocEntry> m_entries;
    Theme m_theme;
    QSet<int> m_highlightedEntries;
};
