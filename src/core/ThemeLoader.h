// src/core/ThemeLoader.h
//
// Spec: specs/模块-app/12-主题插件系统.md
// Spec: specs/横切关注点/30-主题系统.md (INV-6)
// Invariants enforced here: INV-1, INV-2, INV-3, INV-4, INV-8, INV-11, INV-12, INV-13
// Last synced: 2026-04-14
//
// 数据驱动的主题加载器：从 TOML 配置文件构造 Theme。
//
// 解析器支持的 TOML 子集（MVP）：
//   - [section] 与 [section.subsection] 嵌套 table
//   - key = "string"
//   - key = integer / float
//   - key = true / false
//   - 行注释 '#'
//   - 空行
//   - UTF-8 无 BOM 源文件
//
// 不支持（V1 明确放弃）：
//   - 数组 [1, 2, 3]
//   - 多行字符串 """..."""
//   - 内联 table  { a = 1, b = 2 }
//   - 时间/日期字面量
//   - extends 继承机制（V2）
//
// 所有字段错误都收集进 LoadResult::errors（非 fail-fast），
// 未知字段归入 warnings，无法识别的色值落默认值并 warn。
#pragma once

#include "Theme.h"

#include <QList>
#include <QString>
#include <QStringList>

namespace core {

struct ThemeDescriptor {
    QString id;               // 文件名去后缀，如 "light" / "liquid-glass"
    QString displayName;      // [meta].name，缺失时用 id
    QString filePath;         // 绝对路径；内置主题用 ":/themes/xxx.toml"
    bool    isBuiltin = false;
    bool    isDark    = false; // 用于"跟随系统"默认挑选
};

struct LoadResult {
    bool               ok = false;
    Theme              theme;
    QStringList        errors;       // 字段级致命错误（色值解析失败等）
    QStringList        warnings;     // 未知字段、格式建议
    ThemeDescriptor    descriptor;
};

class ThemeLoader {
public:
    // 从 TOML 文本加载。source 参数仅用于错误消息。
    static LoadResult loadFromString(const QString& toml, const QString& source = QStringLiteral("<string>"));

    // 从文件加载（磁盘路径或 Qt 资源路径 ":/..."）。自动识别 UTF-8 BOM 并剥除。
    static LoadResult loadFromFile(const QString& path);

    // 扫描目录下所有 *.toml，返回描述符列表（不实际解析 body 以加快启动）。
    static QList<ThemeDescriptor> scanDirectory(const QString& dirPath, bool markBuiltin);

    // 合并内置主题 + 用户目录主题（去重，用户优先）。
    static QList<ThemeDescriptor> discoverAll(const QString& userDirPath);

    // 返回用户主题目录（不存在则创建并写入 example）。
    static QString userThemeDir();

    // 序列化 Theme 为 TOML 字符串。供导出 / 测试使用。
    static QString serialize(const Theme& theme);

    // Spec: specs/模块-app/12-主题插件系统.md INV-15
    // 当目录为空（不含任何文件）时，从 :/theme_docs/ 注入 template.toml + HOW_TO.md
    // 让用户立即知道如何写自定义主题。
    //
    // 行为：
    //   - 目录非空（任意一个文件）→ 不操作，writtenOut/failedOut 都置空，返回 false
    //   - 目录为空 → 尝试拷贝模板，写入成功的文件名追加到 writtenOut，
    //                失败的追加到 failedOut；只要至少写入了一个就返回 true。
    //   - 目录不存在但 dir 路径合法 → 视为空目录，先 mkpath 再注入。
    //
    // writtenOut / failedOut 可为 nullptr。
    static bool injectThemeTemplatesIfEmpty(const QString& dir,
                                            QStringList* writtenOut = nullptr,
                                            QStringList* failedOut = nullptr);
};

} // namespace core
