#include "TocPanel.h"

#include <QPushButton>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QLabel>
#include <QTimer>

TocPanel::TocPanel(QWidget* parent)
    : QWidget(parent)
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // 标题
    auto* titleLabel = new QLabel(tr("Contents"), this);
    titleLabel->setObjectName("tocTitle");
    titleLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    titleLabel->setContentsMargins(12, 8, 8, 8);
    mainLayout->addWidget(titleLabel);

    // 可滚动的条目列表
    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scrollArea->setFrameShape(QFrame::NoFrame);

    QWidget* listWidget = new QWidget();
    m_listLayout = new QVBoxLayout(listWidget);
    m_listLayout->setContentsMargins(8, 2, 8, 8);
    m_listLayout->setSpacing(1);
    m_listLayout->addStretch();
    m_scrollArea->setWidget(listWidget);

    mainLayout->addWidget(m_scrollArea);

    setTheme(Theme::light());
}

void TocPanel::setEntries(const QVector<TocEntry>& entries)
{
    m_entries = entries;
    buildList();
}

void TocPanel::setHighlightedEntries(const QSet<int>& indices)
{
    m_highlightedEntries = indices;
    buildList();
}

void TocPanel::setTheme(const Theme& theme)
{
    m_theme = theme;

    QColor bg = theme.isDark ? QColor(37, 37, 38) : QColor(245, 245, 245);
    QColor borderColor = theme.isDark ? QColor(60, 63, 65) : QColor(220, 220, 220);
    QColor titleFg = theme.isDark ? QColor(200, 200, 200) : QColor(80, 80, 80);

    setStyleSheet(QString(
        "TocPanel { background: %1; border-left: 1px solid %2; }"
    ).arg(bg.name(), borderColor.name()));

    // 标题样式
    findChild<QLabel*>("tocTitle")->setStyleSheet(QString(
        "QLabel { color: %1; font-size: 14px; font-weight: bold; background: transparent; }"
    ).arg(titleFg.name()));

    // scrollArea 透明
    m_scrollArea->setStyleSheet(QString(
        "QScrollArea { background: transparent; }"
        "QWidget { background: transparent; }"
        "QScrollBar:vertical { width: 5px; background: transparent; }"
        "QScrollBar::handle:vertical { background: %1; border-radius: 2px; min-height: 20px; }"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }"
    ).arg(theme.isDark ? "rgba(255,255,255,50)" : "rgba(0,0,0,30)"));

    buildList();
}

void TocPanel::buildList()
{
    // 清空现有条目（保留 stretch）
    while (m_listLayout->count() > 1) {
        QLayoutItem* item = m_listLayout->takeAt(0);
        delete item->widget();
        delete item;
    }

    QColor fg = m_theme.isDark ? QColor(210, 210, 210) : QColor(50, 50, 50);
    QColor subFg = m_theme.isDark ? QColor(170, 170, 170) : QColor(90, 90, 90);
    QColor hoverBg = m_theme.isDark ? QColor(65, 68, 72) : QColor(220, 224, 230);
    QColor hoverFg = m_theme.previewLink;

    for (int i = 0; i < m_entries.size(); ++i) {
        const auto& entry = m_entries[i];
        QPushButton* btn = new QPushButton(entry.title, m_scrollArea->widget());
        btn->setCursor(Qt::PointingHandCursor);
        btn->setFocusPolicy(Qt::NoFocus);
        btn->setFlat(true);

        int indent = (entry.level - 1) * 14;
        int fontSize = entry.level == 1 ? 14 : 13;
        QColor itemFg = entry.level <= 2 ? fg : subFg;

        QString normalBg = "transparent";
        if (m_highlightedEntries.contains(i)) {
            normalBg = m_theme.previewHighlightToc.name(QColor::HexArgb);
        }

        btn->setStyleSheet(QString(
            "QPushButton {"
            "  padding: 5px 8px 5px %1px;"
            "  color: %2; font-size: %3px;"
            "  border: none; border-radius: 3px; background: %6;"
            "  text-align: left;"
            "}"
            "QPushButton:hover { background: %4; color: %5; }"
        ).arg(8 + indent).arg(itemFg.name()).arg(fontSize)
         .arg(hoverBg.name(), hoverFg.name(), normalBg));

        int sourceLine = entry.sourceLine;
        connect(btn, &QPushButton::clicked, this, [this, sourceLine]() {
            emit headingClicked(sourceLine);
        });

        // 插入在 stretch 之前
        m_listLayout->insertWidget(m_listLayout->count() - 1, btn);
    }
}
