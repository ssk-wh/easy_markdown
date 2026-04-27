#pragma once
//
// Spec: specs/模块-app/07-快捷键弹窗.md
// Invariants enforced here: INV-1, INV-2, INV-3, INV-4
// Depends: specs/横切关注点/30-主题系统.md (INV-1, INV-4)
// Last synced: 2026-04-13

#include <QDialog>
#include "Theme.h"

class QLineEdit;
class QPushButton;
class QTableWidget;

class ShortcutsDialog : public QDialog {
    Q_OBJECT
public:
    // 构造函数强制接收 Theme，不提供默认值（Spec 07 INV-1）。
    explicit ShortcutsDialog(const Theme& theme, QWidget* parent = nullptr);
    void setTheme(const Theme& theme);

private:
    void setupUI();
    void populateShortcuts();
    void applyTheme();
    void filterShortcuts(const QString& text);

    QLineEdit* m_searchEdit;
    QTableWidget* m_table;
    QPushButton* m_closeBtn = nullptr;
    Theme m_theme;

    struct ShortcutInfo {
        QString category;
        QString action;
        QString shortcut;
    };
    QVector<ShortcutInfo> m_shortcuts;
};
