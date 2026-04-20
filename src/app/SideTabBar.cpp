// src/app/SideTabBar.cpp
//
// Spec: (无独立 Spec，作为 Tab 栏位置可调功能的一部分)
// Last synced: 2026-04-20
#include "SideTabBar.h"
#include "Theme.h"

#include <QLabel>
#include <QHBoxLayout>
#include <QScrollBar>
#include <QEvent>

SideTabBar::SideTabBar(QWidget* parent)
    : QWidget(parent)
    , m_fgColor(Qt::black)
{
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    // 标题栏
    m_titleLabel = new QLabel(tr("Recent Documents"), this);
    m_titleLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    m_titleLabel->setContentsMargins(8, 4, 8, 4);
    QFont titleFont = m_titleLabel->font();
    titleFont.setBold(true);
    titleFont.setPointSizeF(titleFont.pointSizeF() * 0.9);
    m_titleLabel->setFont(titleFont);
    layout->addWidget(m_titleLabel);

    m_list = new QListWidget(this);
    m_list->setFrameShape(QFrame::NoFrame);
    m_list->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_list->setSelectionMode(QAbstractItemView::SingleSelection);
    m_list->setDragDropMode(QAbstractItemView::NoDragDrop);
    m_list->setFocusPolicy(Qt::NoFocus);
    m_list->setMouseTracking(true);  // 需要 hover 事件
    layout->addWidget(m_list, 1);

    m_addBtn = new QToolButton(this);
    m_addBtn->setText("+");
    m_addBtn->setToolButtonStyle(Qt::ToolButtonTextOnly);
    m_addBtn->setCursor(Qt::PointingHandCursor);
    m_addBtn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_addBtn->setFixedHeight(28);
    layout->addWidget(m_addBtn);

    connect(m_addBtn, &QToolButton::clicked, this, &SideTabBar::addClicked);

    connect(m_list, &QListWidget::currentRowChanged, this, [this](int row) {
        if (!m_updatingIndex && row >= 0) {
            updateItemStyles();
            emit currentChanged(row);
        }
    });
}

bool SideTabBar::eventFilter(QObject* obj, QEvent* event)
{
    // item widget 的 enter/leave 事件：控制关闭按钮可见性
    if (event->type() == QEvent::Enter || event->type() == QEvent::Leave) {
        auto* widget = qobject_cast<QWidget*>(obj);
        if (widget) {
            auto* closeBtn = widget->findChild<QToolButton*>();
            if (closeBtn)
                closeBtn->setVisible(event->type() == QEvent::Enter);
        }
    }
    return QWidget::eventFilter(obj, event);
}

int SideTabBar::addTab(const QString& text)
{
    auto* item = new QListWidgetItem();
    m_list->addItem(item);
    int row = m_list->count() - 1;
    rebuildItem(row, text);
    return row;
}

void SideTabBar::removeTab(int index)
{
    if (index < 0 || index >= m_list->count()) return;
    m_updatingIndex = true;
    delete m_list->takeItem(index);
    m_updatingIndex = false;
}

void SideTabBar::setTabText(int index, const QString& text)
{
    if (index < 0 || index >= m_list->count()) return;
    // 仅更新 label 文本，不重建整个 widget
    auto* w = m_list->itemWidget(m_list->item(index));
    if (w) {
        auto* label = w->findChild<QLabel*>();
        if (label) label->setText(text);
    }
    updateItemStyles();
}

void SideTabBar::setCurrentIndex(int index)
{
    if (index < 0 || index >= m_list->count()) return;
    m_updatingIndex = true;
    m_list->setCurrentRow(index);
    m_updatingIndex = false;
    updateItemStyles();
}

int SideTabBar::currentIndex() const
{
    return m_list->currentRow();
}

int SideTabBar::count() const
{
    return m_list->count();
}

void SideTabBar::clear()
{
    m_updatingIndex = true;
    m_list->clear();
    m_updatingIndex = false;
}

void SideTabBar::moveTabToTop(int index)
{
    if (index <= 0 || index >= m_list->count()) return;
    m_updatingIndex = true;
    // 取出 item 并移到第一位
    auto* item = m_list->takeItem(index);
    m_list->insertItem(0, item);
    // 重建 item widget（takeItem 会清掉 itemWidget）
    rebuildItem(0, item->data(Qt::UserRole).toString());
    m_list->setCurrentRow(0);
    m_updatingIndex = false;
    updateItemStyles();
    emit tabMoved(index, 0);
}

void SideTabBar::rebuildItem(int row, const QString& text)
{
    auto* item = m_list->item(row);
    if (!item) return;

    // 保存文本到 UserRole，方便 moveTabToTop 后恢复
    item->setData(Qt::UserRole, text);

    auto* widget = new QWidget();
    widget->setMouseTracking(true);
    widget->setAttribute(Qt::WA_Hover, true);
    widget->installEventFilter(this);  // 监听 hover 以控制关闭按钮
    auto* hbox = new QHBoxLayout(widget);
    hbox->setContentsMargins(8, 2, 4, 2);
    hbox->setSpacing(4);

    auto* label = new QLabel(text);
    label->setObjectName("tabLabel");
    label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    label->setMinimumWidth(0);
    hbox->addWidget(label, 1);

    auto* closeBtn = new QToolButton();
    closeBtn->setText(QStringLiteral("\u00D7"));  // ×
    closeBtn->setFixedSize(16, 16);
    closeBtn->setCursor(Qt::PointingHandCursor);
    closeBtn->setAutoRaise(true);
    closeBtn->setStyleSheet("QToolButton { border: none; font-size: 14px; }"
                            "QToolButton:hover { background: rgba(0,0,0,0.1); border-radius: 3px; }");
    closeBtn->setVisible(false);  // 默认隐藏，hover 时显示
    hbox->addWidget(closeBtn);

    connect(closeBtn, &QToolButton::clicked, this, [this, item]() {
        int r = m_list->row(item);
        if (r >= 0) emit tabCloseRequested(r);
    });

    item->setSizeHint(QSize(0, 28));
    m_list->setItemWidget(item, widget);
}

void SideTabBar::updateItemStyles()
{
    int current = m_list->currentRow();
    for (int i = 0; i < m_list->count(); ++i) {
        auto* w = m_list->itemWidget(m_list->item(i));
        if (!w) continue;
        auto* label = w->findChild<QLabel*>("tabLabel");
        if (!label) continue;
        QFont f = label->font();
        f.setBold(i == current);
        label->setFont(f);
    }
}

void SideTabBar::setTheme(const Theme& theme)
{
    m_fgColor = theme.editorFg;

    // hover 项 padding-left 增加模拟文字右移效果
    QString listStyle = QStringLiteral(
        "QListWidget { background: %1; border: none; outline: none; }"
        "QListWidget::item { color: %2; padding: 0px; border-left: 3px solid transparent; }"
        "QListWidget::item:selected { background: %3; border-left: 3px solid %5; }"
        "QListWidget::item:hover:!selected { background: %4; padding-left: 4px; }"
        "QListWidget::item:hover:selected { padding-left: 4px; }"
    ).arg(theme.editorBg.name(),
          theme.editorFg.name(),
          theme.editorGutterBg.name(),
          theme.hoverBgCss(),
          theme.accentColor.name());
    m_list->setStyleSheet(listStyle);

    // 同步 label 颜色
    for (int i = 0; i < m_list->count(); ++i) {
        auto* w = m_list->itemWidget(m_list->item(i));
        if (!w) continue;
        auto* label = w->findChild<QLabel*>("tabLabel");
        if (label)
            label->setStyleSheet(QStringLiteral("color: %1;").arg(theme.editorFg.name()));
    }

    // 标题栏样式：底部 1px 分割线
    m_titleLabel->setStyleSheet(QStringLiteral(
        "QLabel { background: %1; color: %2; border-bottom: 1px solid %3; }"
    ).arg(theme.editorBg.name(),
          theme.editorFg.name(),
          theme.isDark ? "rgba(255,255,255,0.1)" : "rgba(0,0,0,0.1)"));

    // + 按钮样式
    QString btnStyle = QStringLiteral(
        "QToolButton { background: %1; color: %2; border: none; font-size: 16px; }"
        "QToolButton:hover { background: %3; }"
    ).arg(theme.editorBg.name(),
          theme.editorFg.name(),
          theme.hoverBgCss());
    m_addBtn->setStyleSheet(btnStyle);

    updateItemStyles();
}
