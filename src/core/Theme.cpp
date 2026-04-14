// Spec: specs/模块-preview/10-Frontmatter渲染.md §4.4 §5.4 §5.5
// Spec: specs/横切关注点/30-主题系统.md （新增 accentColor / frontmatter* 字段）
// Spec: specs/模块-app/12-主题插件系统.md （内置主题走 ThemeLoader）
// Last synced: 2026-04-14
#include "Theme.h"
#include "ThemeLoader.h"

#include <QDebug>
#include <QGuiApplication>
#include <QPalette>

namespace {

// Spec §5.4：RGB 线性混合（不是 alpha 合成），避免与选区/marking 叠加出现双重混合
QColor blendColor(const QColor& accent, const QColor& bg, qreal accentRatio)
{
    const qreal r = accent.redF()   * accentRatio + bg.redF()   * (1.0 - accentRatio);
    const qreal g = accent.greenF() * accentRatio + bg.greenF() * (1.0 - accentRatio);
    const qreal b = accent.blueF()  * accentRatio + bg.blueF()  * (1.0 - accentRatio);
    return QColor::fromRgbF(qBound(0.0, r, 1.0),
                            qBound(0.0, g, 1.0),
                            qBound(0.0, b, 1.0));
}

// Spec §5.5：从系统 palette 取强调色；palette 全灰时 fallback #0078D4
QColor resolveSystemAccent()
{
    const QColor kFallback("#0078D4");
    if (!QGuiApplication::instance())
        return kFallback;
    const QPalette pal = QGuiApplication::palette();
    const QColor hi = pal.color(QPalette::Highlight);
    if (!hi.isValid())
        return kFallback;
    // 判断是否"全灰"：R=G=B 且饱和度极低
    if (hi.red() == hi.green() && hi.green() == hi.blue())
        return kFallback;
    if (hi.saturation() < 20)
        return kFallback;
    return hi;
}

// Theme.h 里 frontmatter/accent 字段的默认值（用于判定 TOML 是否显式声明了）
const QColor kDefaultAccent          ("#0078D4");
const QColor kDefaultFrontmatterBg   ("#E5F1FB");
const QColor kDefaultFrontmatterBorder("#4DA3E1");
const QColor kDefaultFrontmatterKey  ("#1F5A8A");
const QColor kDefaultFrontmatterValue("#333333");

// 仅在 TOML 未显式声明对应字段时，按"accent + previewBg 线性混合"算法派生。
// 判定规则：字段值 == Theme.h 默认 ⇒ 未声明 ⇒ 派生；否则保留 TOML 的值。
//
// Bug 历史：早期版本无条件覆盖 TOML 里的 frontmatter_* 字段，导致墨禅主题的
// frontmatterValueForeground 被强制设成 previewCodeFg（浅奶色反色），与
// 浅奶 background 对比度极低，value 看不清。
void applyFrontmatterColors(Theme& t, bool themeDeclaredAccent)
{
    if (!themeDeclaredAccent)
        t.accentColor = resolveSystemAccent();

    // Spec §5.4/§8.5：背景用 accent 与 previewBg 线性混合。Spec 建议 0.5/0.7，
    // 实测深色主题下 0.5 视觉过重，浅色主题下 0.5 可接受。按 §8.5 建议做主题差异化。
    if (t.frontmatterBackground == kDefaultFrontmatterBg) {
        t.frontmatterBackground = t.isDark
            ? blendColor(t.accentColor, t.previewBg, 0.22)
            : blendColor(t.accentColor, t.previewBg, 0.12);
    }
    if (t.frontmatterBorder == kDefaultFrontmatterBorder) {
        t.frontmatterBorder = t.isDark
            ? blendColor(t.accentColor, t.previewBg, 0.60)
            : blendColor(t.accentColor, t.previewBg, 0.50);
    }
    // Key 偏 accent：accent 与 previewFg 线性混合，保持可读
    if (t.frontmatterKeyForeground == kDefaultFrontmatterKey)
        t.frontmatterKeyForeground = blendColor(t.accentColor, t.previewFg, 0.65);
    // Value 默认与 codeFg 一致（但墨禅等反色代码块主题需显式在 TOML 声明 frontmatter_value）
    if (t.frontmatterValueForeground == kDefaultFrontmatterValue)
        t.frontmatterValueForeground = t.previewCodeFg;
}

// 把 LoadResult 转成最终 Theme：补全 frontmatter / accent 等派生字段。
// Theme 默认值已是浅色；如果 TOML 没覆盖某字段，保持默认。
Theme finalizeTheme(core::LoadResult& r)
{
    Theme t = r.theme;
    // 如果 TOML 文件显式声明了 accent/frontmatter 字段，就不覆盖；
    // 判定方式：对比 accentColor 是否等于 Theme 默认 #0078D4。
    // 由于 #0078D4 恰好是 fallback，这里直接 always-recompute 并允许 TOML 覆写。
    const bool themeDeclaredAccent = (t.accentColor != QColor("#0078D4"));
    applyFrontmatterColors(t, themeDeclaredAccent);
    return t;
}

// 缓存内置主题，避免每次切换都解析 TOML
Theme loadBuiltin(const QString& id)
{
    const QString path = QStringLiteral(":/themes/") + id + QStringLiteral(".toml");
    core::LoadResult r = core::ThemeLoader::loadFromFile(path);
    if (!r.ok) {
        for (const QString& e : r.errors)
            qWarning() << "[Theme]" << id << "load error:" << e;
    }
    for (const QString& w : r.warnings)
        qDebug() << "[Theme]" << id << "warning:" << w;
    return finalizeTheme(r);
}

} // namespace

Theme Theme::light() {
    static Theme cached = loadBuiltin(QStringLiteral("light"));
    return cached;
}

Theme Theme::dark() {
    static Theme cached = loadBuiltin(QStringLiteral("dark"));
    return cached;
}

Theme Theme::byId(const QString& id) {
    // 快速路径：内置
    if (id == QLatin1String("light")) return Theme::light();
    if (id == QLatin1String("dark"))  return Theme::dark();

    // 其他内置 + 用户主题：每次都解析（避免缓存膨胀 + 支持重新扫描）
    const QString userDir = core::ThemeLoader::userThemeDir();
    const auto all = core::ThemeLoader::discoverAll(userDir);
    for (const auto& d : all) {
        if (d.id == id) {
            core::LoadResult r = core::ThemeLoader::loadFromFile(d.filePath);
            if (r.ok) return finalizeTheme(r);
            for (const QString& e : r.errors)
                qWarning() << "[Theme] byId" << id << "error:" << e;
            break;
        }
    }
    qWarning() << "[Theme] theme id not found, falling back to light:" << id;
    return Theme::light();
}

QStringList Theme::availableIds() {
    QStringList ids;
    const QString userDir = core::ThemeLoader::userThemeDir();
    const auto all = core::ThemeLoader::discoverAll(userDir);
    for (const auto& d : all) ids << d.id;
    return ids;
}
