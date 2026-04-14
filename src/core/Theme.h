// src/core/Theme.h
//
// Spec: specs/横切关注点/30-主题系统.md
// Spec: specs/模块-app/12-主题插件系统.md
// Invariants enforced here: INV-1 (唯一数据源), INV-6 (可从配置文件构造)
// Last synced: 2026-04-14
#pragma once
#include <QColor>
#include <QString>
#include <QStringList>

struct Theme {
    QString name;
    bool isDark = false;

    // 编辑器颜色
    QColor editorBg = QColor("#FFFFFF");
    QColor editorFg = QColor("#333333");
    QColor editorCurrentLine = QColor("#F5F5F5");
    QColor editorSelection = QColor("#B5D5FF");
    QColor editorLineNumber = QColor("#999999");
    QColor editorLineNumberActive = QColor("#333333");
    QColor editorGutterBg = QColor("#F0F0F0");
    QColor editorCursor = QColor("#333333");
    QColor editorSearchMatch = QColor(255, 235, 59, 220);  // 亮黄色（alpha 220 保证明显）
    QColor editorSearchMatchCurrent = QColor(255, 140, 0, 240);  // 当前匹配（饱和橙色，更明显）
    QColor editorGutterLine = QColor("#E0E0E0");
    QColor editorPreeditBg = QColor(255, 255, 200);

    // 语法高亮颜色
    QColor syntaxHeading = QColor("#1A237E");
    QColor syntaxCode = QColor("#C62828");
    QColor syntaxCodeBg = QColor("#F5F5F5");
    QColor syntaxCodeBlock = QColor("#2E7D32");
    QColor syntaxCodeBlockBg = QColor("#F8F8F8");
    QColor syntaxLink = QColor("#0366D6");
    QColor syntaxList = QColor("#6A1B9A");
    QColor syntaxBlockQuote = QColor("#757575");
    QColor syntaxFence = QColor("#999999");
    QColor syntaxFenceBg = QColor("#F0F0F0");

    // 预览颜色
    QColor previewBg = QColor("#FFFFFF");
    QColor previewFg = QColor("#333333");
    QColor previewHeading = QColor("#1A1A1A");
    QColor previewLink = QColor("#0366D6");
    QColor previewCodeBg = QColor("#F6F8FA");
    QColor previewCodeFg = QColor("#333333");
    QColor previewCodeBorder = QColor("#E1E4E8");
    QColor previewBlockQuoteBorder = QColor("#DFE2E5");
    QColor previewBlockQuoteBg = QColor("#F8F8F8");
    QColor previewTableBorder = QColor("#DFE2E5");
    QColor previewTableHeaderBg = QColor("#F6F8FA");
    QColor previewHr = QColor("#E0E0E0");
    QColor previewHeadingSeparator = QColor("#EAECEF");
    QColor previewImagePlaceholderBg = QColor("#F0F0F0");
    QColor previewImagePlaceholderBorder = QColor("#CCCCCC");
    QColor previewImagePlaceholderText = QColor("#999999");
    QColor previewImageErrorBg = QColor("#FFF0F0");
    QColor previewImageErrorBorder = QColor("#E57373");
    QColor previewImageErrorText = QColor("#C62828");
    QColor previewImageInfoText = QColor("#666666");
    QColor previewHighlight = QColor(255, 235, 59, 180);     // 标记高亮（浅色）
    QColor previewHighlightToc = QColor(255, 235, 59, 120);  // TOC标记（浅色）

    // Spec: specs/模块-preview/10-Frontmatter渲染.md §4.4
    // accentColor 优先取 QGuiApplication::palette().color(QPalette::Highlight)，
    // 全灰时 fallback 为 #0078D4；frontmatter* 字段由 accent 与 previewBg 线性混合得到
    QColor accentColor = QColor("#0078D4");
    QColor frontmatterBackground = QColor("#E5F1FB");   // blend(accent, previewBg, 0.5)
    QColor frontmatterBorder = QColor("#4DA3E1");       // blend(accent, previewBg, 0.7)
    QColor frontmatterKeyForeground = QColor("#1F5A8A");     // 偏 accent
    QColor frontmatterValueForeground = QColor("#333333");   // 与 previewCodeFg 一致

    // 内置主题（V1 从 :/themes/*.toml 加载，保持 API 向后兼容）
    // Spec: specs/模块-app/12-主题插件系统.md 内置主题通过 ThemeLoader
    static Theme light();
    static Theme dark();

    // V1 新增：按主题 ID 加载（内置 or 用户目录）。
    // 找不到时返回 light() 并在 appLog 记录 warning。
    // 传 "light" / "dark" / "liquid-glass" 可直接拿到内置主题。
    static Theme byId(const QString& id);

    // 可用主题 ID 列表（内置 + 用户目录 .toml）
    static QStringList availableIds();
};
