// src/app/SideTabBar.h
//
// 侧边 Tab 栏：竖向排列、文字横向显示的 Tab 列表，附带「+」新建按钮。
// 作为 TabBarWithAdd 的侧边替代视图，通过 MainWindow 与 QStackedWidget 同步。
#pragma once

#include <QWidget>
#include <QListWidget>
#include <QToolButton>
#include <QVBoxLayout>
#include <QLabel>

struct Theme;

class SideTabBar : public QWidget
{
    Q_OBJECT
public:
    explicit SideTabBar(QWidget* parent = nullptr);

    int  addTab(const QString& text);
    void removeTab(int index);
    void setTabText(int index, const QString& text);
    void setCurrentIndex(int index);
    int  currentIndex() const;
    int  count() const;
    void clear();

    // 将指定 tab 移到第一位并选中
    void moveTabToTop(int index);

    void setTheme(const Theme& theme);

signals:
    void currentChanged(int index);
    void tabCloseRequested(int index);
    void addClicked();
    void tabMoved(int from, int to);  // 通知宿主同步 tab 顺序

protected:
    bool eventFilter(QObject* obj, QEvent* event) override;

private:
    QLabel* m_titleLabel;   // "最近使用文档" 标题
    QListWidget* m_list;
    QToolButton* m_addBtn;
    bool m_updatingIndex = false;  // 防止信号递归
    Theme const* m_cachedTheme = nullptr;  // 缓存主题指针用于样式刷新
    QColor m_fgColor;  // 缓存的前景色
    bool m_isDark = false;  // 缓存当前是否深色主题

    void rebuildItem(int row, const QString& text);
    void updateItemStyles();  // 刷新所有 item 的选中/未选中样式
};
