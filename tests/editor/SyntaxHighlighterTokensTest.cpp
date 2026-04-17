// tests/editor/SyntaxHighlighterTokensTest.cpp
//
// Spec: specs/模块-editor/README.md
// 验收：T-STRIKE-1, T-BOLDITALIC-1, T-PRIORITY-1
// Plan: plans/归档/2026-04-15-编辑器染色补strike和bold-italic.md

#include "SyntaxHighlighter.h"
#include "Theme.h"

#include <QFont>
#include <QGuiApplication>
#include <QTextCharFormat>
#include <gtest/gtest.h>

namespace {

// 找到覆盖 needle（子串）的第一个 HighlightToken
const HighlightToken* findTokenCovering(const QVector<HighlightToken>& tokens,
                                        const QString& full,
                                        const QString& needle)
{
    int pos = full.indexOf(needle);
    if (pos < 0) return nullptr;
    for (const auto& t : tokens) {
        if (t.start <= pos && pos + needle.length() <= t.start + t.length) {
            return &t;
        }
    }
    return nullptr;
}

} // namespace

TEST(SyntaxHighlighterTokens, T_STRIKE_1_Strikethrough) {
    SyntaxHighlighter h;
    h.setTheme(Theme::light());
    h.setLineCount(1);

    const QString line = QStringLiteral("a ~~foo~~ b");
    auto tokens = h.highlightLine(0, line);

    const HighlightToken* tok = findTokenCovering(tokens, line, QStringLiteral("~~foo~~"));
    ASSERT_NE(tok, nullptr) << "missing strikethrough token";
    EXPECT_TRUE(tok->format.fontStrikeOut())
        << "strikethrough token must have fontStrikeOut==true";
}

TEST(SyntaxHighlighterTokens, T_BOLDITALIC_1_BoldItalic) {
    SyntaxHighlighter h;
    h.setTheme(Theme::light());
    h.setLineCount(1);

    const QString line = QStringLiteral("pre ***foo*** post");
    auto tokens = h.highlightLine(0, line);

    const HighlightToken* tok = findTokenCovering(tokens, line, QStringLiteral("***foo***"));
    ASSERT_NE(tok, nullptr) << "missing bold-italic token";
    // 粗斜体必须同时满足两个条件
    EXPECT_GE(tok->format.fontWeight(), static_cast<int>(QFont::Bold))
        << "bold-italic token must be bold";
    EXPECT_TRUE(tok->format.fontItalic())
        << "bold-italic token must be italic";
}

// ***foo*** 不应被 bold / italic 正则额外覆盖产生重复 token
TEST(SyntaxHighlighterTokens, T_BOLDITALIC_NoDuplicate) {
    SyntaxHighlighter h;
    h.setTheme(Theme::light());
    h.setLineCount(1);

    const QString line = QStringLiteral("***foo***");
    auto tokens = h.highlightLine(0, line);

    int boldItalicCount = 0, boldOnlyCount = 0, italicOnlyCount = 0;
    for (const auto& t : tokens) {
        bool bold = t.format.fontWeight() >= static_cast<int>(QFont::Bold);
        bool italic = t.format.fontItalic();
        if (bold && italic) ++boldItalicCount;
        else if (bold) ++boldOnlyCount;
        else if (italic) ++italicOnlyCount;
    }
    EXPECT_EQ(boldItalicCount, 1);
    EXPECT_EQ(boldOnlyCount, 0);
    EXPECT_EQ(italicOnlyCount, 0);
}

// T-PRIORITY-1：inline code 范围内的 ~~ / *** 不应触发删除线/粗斜体
TEST(SyntaxHighlighterTokens, T_PRIORITY_1_InsideInlineCode) {
    SyntaxHighlighter h;
    h.setTheme(Theme::light());
    h.setLineCount(1);

    // 整段 inline code 里含 ~~foo~~ —— 不应再产生 strikethrough token
    const QString line = QStringLiteral("pre `~~foo~~ and ***bar***` post");
    auto tokens = h.highlightLine(0, line);

    // code 范围：第一个 ` 到最后一个 `
    int codeStart = line.indexOf(QLatin1Char('`'));
    int codeEnd = line.lastIndexOf(QLatin1Char('`')) + 1;
    ASSERT_GE(codeStart, 0);
    ASSERT_GT(codeEnd, codeStart);

    int strikeCount = 0, boldItalicCount = 0;
    for (const auto& t : tokens) {
        if (t.start >= codeStart && t.start + t.length <= codeEnd) {
            if (t.format.fontStrikeOut()) ++strikeCount;
            if (t.format.fontWeight() >= static_cast<int>(QFont::Bold) && t.format.fontItalic())
                ++boldItalicCount;
        }
    }
    EXPECT_EQ(strikeCount, 0) << "strikethrough inside inline code must not fire";
    EXPECT_EQ(boldItalicCount, 0) << "bold-italic inside inline code must not fire";

    // 验证只有 1 个 code token 覆盖 `~~foo~~ and ***bar***`
    bool codeTokenFound = false;
    for (const auto& t : tokens) {
        const QString slice = line.mid(t.start, t.length);
        if (slice.startsWith(QLatin1Char('`')) && slice.endsWith(QLatin1Char('`'))) {
            codeTokenFound = true;
        }
    }
    EXPECT_TRUE(codeTokenFound);
}

int main(int argc, char** argv) {
    QGuiApplication app(argc, argv);
    Q_INIT_RESOURCE(resources);
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
