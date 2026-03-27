#pragma once

#include <QWidget>
#include <QVector>
#include <QString>
#include "Theme.h"

class QPushButton;
class QScrollArea;
class QVBoxLayout;
class QLabel;

// 自绘背景的面板
class TocPanelBackground : public QWidget {
    Q_OBJECT
public:
    using QWidget::QWidget;
    void setBgColor(const QColor& c) { m_bg = c; update(); }
    void setBorderColor(const QColor& c) { m_border = c; update(); }
protected:
    void paintEvent(QPaintEvent*) override;
private:
    QColor m_bg = QColor(0, 0, 0, 18);
    QColor m_border = QColor(0, 0, 0, 30);
};

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
    void reposition();

signals:
    void headingClicked(int sourceLine);

protected:
    bool eventFilter(QObject* obj, QEvent* event) override;

private:
    void toggle();
    void buildList();
    void hidePanel();

    QPushButton* m_toggleBtn;
    TocPanelBackground* m_panel;
    QScrollArea* m_scrollArea;
    QVBoxLayout* m_listLayout;
    QVector<TocEntry> m_entries;
    Theme m_theme;
    bool m_panelVisible = false;
};
