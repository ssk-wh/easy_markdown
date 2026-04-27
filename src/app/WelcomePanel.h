// src/app/WelcomePanel.h
//
// Spec: specs/模块-app/22-空白引导页.md
// Invariants enforced here: INV-EMPTY-WELCOME-MUTUAL, INV-EMPTY-I18N, INV-EMPTY-THEME
// Last synced: 2026-04-27
#pragma once

#include <QWidget>
#include "Theme.h"

class QLabel;
class QPushButton;

/// 空白引导面板：当 m_tabs 为空（启动无可恢复内容、关闭最后一个 Tab）时显示，
/// 提供主动入口（打开文件 / 打开文件夹 / 新建文件）。
class WelcomePanel : public QWidget {
    Q_OBJECT
public:
    explicit WelcomePanel(QWidget* parent = nullptr);

    /// 应用主题：背景、前景、按钮、accent 颜色全部来自 Theme（INV-EMPTY-THEME）
    void setTheme(const Theme& theme);

signals:
    /// 用户点击「打开文件...」按钮 → MainWindow::onOpenFile
    void openFileClicked();
    /// 用户点击「打开文件夹...」按钮 → MainWindow::onOpenFolder
    void openFolderClicked();
    /// 用户点击「新建文件」按钮 → MainWindow::onNewFile
    void newFileClicked();

protected:
    void changeEvent(QEvent* event) override;  // QEvent::LanguageChange → 重译

private:
    void retranslateUi();   // i18n：所有用户可见文本走 tr()
    void rebuildStyles();   // 主题切换时重新生成 stylesheet

    QLabel* m_titleLabel = nullptr;
    QLabel* m_subtitleLabel = nullptr;
    QPushButton* m_openFileBtn = nullptr;
    QPushButton* m_openFolderBtn = nullptr;
    QPushButton* m_newFileBtn = nullptr;
    QLabel* m_hintLabel = nullptr;

    Theme m_theme;
};
