// tests/editor/SyntaxHighlighterThemeCacheTest.cpp
//
// 回归测试：主题切换后 SyntaxHighlighter::m_cache 必须 invalidate，
// 否则旧主题 token 的 QTextCharFormat::background() 会被复用，在新主题下看到
// "残留行背景"。
//
// Spec: specs/模块-app/12-主题插件系统.md（V1 落地后修复）
// Bug 发现时间：2026-04-14（用户在主题插件系统 V1 落地后切换主题时报告）

#include "SyntaxHighlighter.h"
#include "Theme.h"

#include <QGuiApplication>
#include <QTextCharFormat>
#include <gtest/gtest.h>

// helper：从一组 tokens 里找 ` ` 行内代码 token 的 background 色
static QColor findCodeBg(const QVector<HighlightToken>& tokens, const QString& text) {
    for (const auto& t : tokens) {
        if (t.start >= 0 && t.start + t.length <= text.length()) {
            const QString slice = text.mid(t.start, t.length);
            if (slice.startsWith(QLatin1Char('`')) && slice.endsWith(QLatin1Char('`'))) {
                return t.format.background().color();
            }
        }
    }
    return QColor();
}

TEST(SyntaxHighlighterThemeCache, SwitchThemeClearsCachedTokens) {
    SyntaxHighlighter h;
    h.setLineCount(1);

    const QString line = QStringLiteral("this is `inline code` sample");

    // 1) 初始 Light 主题
    h.setTheme(Theme::light());
    auto tokens1 = h.highlightLine(0, line);
    QColor bgLight = findCodeBg(tokens1, line);
    ASSERT_TRUE(bgLight.isValid()) << "light code token not found";

    // 2) 切 arctic-frost（code_background = #E7EFFB 浅冰蓝）
    Theme arctic = Theme::byId(QStringLiteral("arctic-frost"));
    h.setTheme(arctic);
    auto tokens2 = h.highlightLine(0, line);
    QColor bgArctic = findCodeBg(tokens2, line);
    ASSERT_TRUE(bgArctic.isValid()) << "arctic code token not found";
    EXPECT_NE(bgLight.name(), bgArctic.name())
        << "arctic code bg should differ from light";

    // 3) 切回 Dark（code_background = #2D2D2D）
    h.setTheme(Theme::dark());
    auto tokens3 = h.highlightLine(0, line);
    QColor bgDark = findCodeBg(tokens3, line);
    ASSERT_TRUE(bgDark.isValid()) << "dark code token not found";

    // 关键断言：dark 主题下 code bg 必须是 #2D2D2D，不能是 arctic 的 #E7EFFB
    EXPECT_EQ(bgDark.name().toLower(), QStringLiteral("#2d2d2d"))
        << "expected dark bg #2D2D2D, got " << bgDark.name().toStdString()
        << " — cache not invalidated on theme switch";
    EXPECT_NE(bgDark.name(), bgArctic.name())
        << "dark still showing arctic cached bg (cache not cleared)";
}

int main(int argc, char** argv) {
    // QGuiApplication 需要：Theme::light/dark 内部会访问 QGuiApplication::palette()
    QGuiApplication app(argc, argv);
    Q_INIT_RESOURCE(resources);
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
