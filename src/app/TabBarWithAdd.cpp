// Spec: specs/模块-app/04-窗口焦点管理.md
// Spec: specs/模块-app/12-主题插件系统.md
// Last synced: 2026-04-14
#include "TabBarWithAdd.h"

#include <QMouseEvent>
#include <QPainter>
#include <QStyleOptionTab>

namespace {
constexpr int kAddBtnWidth = 32;
constexpr int kAddBtnIconSize = 12;      // + 图标的"臂长"
constexpr int kAddBtnGap = 2;            // 紧贴最后 tab 后的小间距
}

TabBarWithAdd::TabBarWithAdd(QWidget* parent)
    : QTabBar(parent)
{
    setMouseTracking(true);
    // Qt 默认 palette()，通常能在没显式 setAddButtonColors 时也给出合理默认
    m_addFg = palette().color(QPalette::WindowText);
    m_addHoverBg = QColor(0, 0, 0, 26);
}

void TabBarWithAdd::setAddButtonColors(const QColor& fg, const QColor& hoverBg)
{
    m_addFg = fg;
    m_addHoverBg = hoverBg;
    update();
}

QRect TabBarWithAdd::addButtonRect() const
{
    if (count() == 0) {
        // 没有 tab 时放在最左侧一点（其实 count==0 下 TabBar 不太可能出现，但防御）
        return QRect(kAddBtnGap, 0, kAddBtnWidth, height());
    }
    QRect lastTab = tabRect(count() - 1);
    int x = lastTab.right() + kAddBtnGap;
    int h = lastTab.height();
    int y = lastTab.y();
    return QRect(x, y, kAddBtnWidth, h);
}

QSize TabBarWithAdd::sizeHint() const
{
    QSize base = QTabBar::sizeHint();
    // 给 + 按钮预留水平空间，避免 Tab 太多时按钮被截掉
    return QSize(base.width() + kAddBtnWidth + kAddBtnGap, base.height());
}

QSize TabBarWithAdd::minimumTabSizeHint(int index) const
{
    return QTabBar::minimumTabSizeHint(index);
}

void TabBarWithAdd::paintEvent(QPaintEvent* e)
{
    // 先让 Qt 画所有 Tab
    QTabBar::paintEvent(e);

    if (count() == 0) return;

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);
    const QRect r = addButtonRect();

    // hover 态：填浅色 rounded rect
    if (m_addHover) {
        QColor bg = m_addHoverBg;
        if (!bg.isValid()) bg = QColor(0, 0, 0, 26);
        p.setPen(Qt::NoPen);
        p.setBrush(bg);
        // 缩小 2px 作为 hit-rect 内的 visual pad
        QRect visRect = r.adjusted(4, 4, -4, -4);
        p.drawRoundedRect(visRect, 4, 4);
    }

    // 画「+」十字
    QColor fg = m_addFg.isValid() ? m_addFg : palette().color(QPalette::WindowText);
    QPen pen(fg);
    pen.setWidthF(1.4);
    pen.setCapStyle(Qt::RoundCap);
    p.setPen(pen);

    const QPoint center = r.center();
    const int half = kAddBtnIconSize / 2;
    p.drawLine(center.x() - half, center.y(),
               center.x() + half, center.y());
    p.drawLine(center.x(), center.y() - half,
               center.x(), center.y() + half);
}

void TabBarWithAdd::mouseMoveEvent(QMouseEvent* e)
{
    const bool hover = addButtonRect().contains(e->pos());
    if (hover != m_addHover) {
        m_addHover = hover;
        // 悬停在 + 按钮时改手型光标；离开时交还 QTabBar 自己（它会按 tab 的 hit 决定）
        setCursor(hover ? Qt::PointingHandCursor : Qt::ArrowCursor);
        update();
    }
    QTabBar::mouseMoveEvent(e);
}

void TabBarWithAdd::mousePressEvent(QMouseEvent* e)
{
    if (e->button() == Qt::LeftButton && addButtonRect().contains(e->pos())) {
        emit addClicked();
        e->accept();
        return;  // 不转给 QTabBar，避免误点到最后 Tab 空白触发
    }
    QTabBar::mousePressEvent(e);
}

void TabBarWithAdd::leaveEvent(QEvent* e)
{
    if (m_addHover) {
        m_addHover = false;
        setCursor(Qt::ArrowCursor);
        update();
    }
    QTabBar::leaveEvent(e);
}

// ---- TabWidgetWithAdd ----
TabWidgetWithAdd::TabWidgetWithAdd(QWidget* parent)
    : QTabWidget(parent)
    , m_bar(new TabBarWithAdd(this))
{
    setTabBar(m_bar);  // protected —— 子类中可用
}
