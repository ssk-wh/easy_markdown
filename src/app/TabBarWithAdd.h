// src/app/TabBarWithAdd.h
//
// Spec: specs/模块-app/04-窗口焦点管理.md — Tab 栏「+」按钮紧贴最后一个 Tab
// Spec: specs/模块-app/12-主题插件系统.md — 按钮视觉从 Theme 字段派生
//
// 自定义 QTabBar 子类：在最后一个 Tab 右侧绘制一个「+」按钮（Chrome/Edge 风格）。
// 点击按钮 emit addClicked()，宿主连接到 MainWindow::onNewFile。
// 不改动 Tab 本身的行为（拖拽、右键菜单、关闭按钮），只额外画一个 hit 区。
#pragma once

#include <QTabBar>
#include <QTabWidget>

class TabBarWithAdd : public QTabBar
{
    Q_OBJECT
public:
    explicit TabBarWithAdd(QWidget* parent = nullptr);

    // 额外的水平空间，用于容纳「+」按钮（宽度 ~32px + 内边距）
    QSize sizeHint() const override;
    QSize minimumTabSizeHint(int index) const;

    // 让 + 按钮跟随主题变色（可选，调用方设置当前 accent/fg/hover）
    void setAddButtonColors(const QColor& fg, const QColor& hoverBg);

signals:
    void addClicked();

protected:
    void paintEvent(QPaintEvent* e) override;
    void mouseMoveEvent(QMouseEvent* e) override;
    void mousePressEvent(QMouseEvent* e) override;
    void leaveEvent(QEvent* e) override;

private:
    // 「+」按钮的几何区域：紧贴最后一个 Tab 右侧，宽度固定
    QRect addButtonRect() const;

    bool m_addHover = false;
    QColor m_addFg;       // + 图标颜色（默认 palette().windowText）
    QColor m_addHoverBg;  // hover 态底色（默认半透明）
};

// QTabWidget 子类——仅用于在构造时 setTabBar(TabBarWithAdd)。
// QTabWidget::setTabBar 是 protected，必须通过子类化访问。
class TabWidgetWithAdd : public QTabWidget
{
    Q_OBJECT
public:
    explicit TabWidgetWithAdd(QWidget* parent = nullptr);
    TabBarWithAdd* customTabBar() const { return m_bar; }
private:
    TabBarWithAdd* m_bar;
};
