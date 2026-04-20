// src/app/FolderPanel.h
//
// Plan: plans/2026-04-14-文件夹侧栏.md
// 左侧文件夹面板：QTreeView + QFileSystemModel，显示目录下的 Markdown 文件
#pragma once

#include <QWidget>
#include "Theme.h"

class QTreeView;
class QFileSystemModel;
class QLabel;
class QToolButton;

class FolderPanel : public QWidget {
    Q_OBJECT
public:
    explicit FolderPanel(QWidget* parent = nullptr);

    void setRootPath(const QString& path);
    QString rootPath() const { return m_rootPath; }
    void setTheme(const Theme& theme);
    void clearRoot();
    void selectFile(const QString& filePath);

signals:
    // 单击文件 → 用当前 Tab 打开
    void fileClicked(const QString& filePath);
    // 双击文件 → 新 Tab 打开
    void fileDoubleClicked(const QString& filePath);

protected:
    void contextMenuEvent(QContextMenuEvent* event) override;

private:
    void onItemClicked(const QModelIndex& index);
    void onItemDoubleClicked(const QModelIndex& index);
    void applyThemeStyles();

    // 右键菜单操作
    void newFile(const QString& parentDir);
    void newFolder(const QString& parentDir);
    void renameItem(const QString& path);
    void deleteItem(const QString& path);
    void revealInExplorer(const QString& path);

    QLabel* m_titleLabel = nullptr;
    QToolButton* m_closeBtn = nullptr;
    QTreeView* m_treeView = nullptr;
    QFileSystemModel* m_model = nullptr;
    QString m_rootPath;
    Theme m_theme;
};
