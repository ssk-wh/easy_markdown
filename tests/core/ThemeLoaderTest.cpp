// tests/core/ThemeLoaderTest.cpp
//
// Spec: specs/模块-app/12-主题插件系统.md
// Last synced: 2026-04-14
//
// ThemeLoader 的单元测试。覆盖：
//   T1: 解析 section + key = value 基本结构
//   T2: 色值格式（#RGB / #RRGGBB / #RRGGBBAA / rgb() / rgba()）
//   T3: 注释与空行被正确忽略
//   T4: 非法色值收集到 errors 且不影响其他字段
//   T5: 未知字段收集到 warnings
//   T6: 内置 light/dark/liquid-glass 资源均能无错加载
//   T7: 序列化后重新解析 round-trip 保持关键字段相等

#include "core/Theme.h"
#include "core/ThemeLoader.h"

#include <QGuiApplication>
#include <QtCore/QString>
#include <gtest/gtest.h>

using core::ThemeLoader;
using core::LoadResult;

TEST(ThemeLoaderTest, T1_ParsesBasicSectionAndKeyValue) {
    const QString toml = QString::fromUtf8(
        "[meta]\n"
        "name = \"Ocean\"\n"
        "id = \"ocean\"\n"
        "is_dark = true\n"
        "\n"
        "[editor]\n"
        "background = \"#112233\"\n"
        "foreground = \"#FFEEDD\"\n"
    );
    LoadResult r = ThemeLoader::loadFromString(toml);
    ASSERT_TRUE(r.errors.isEmpty()) << r.errors.join(";").toStdString();
    EXPECT_EQ(r.theme.name.toStdString(), "Ocean");
    EXPECT_TRUE(r.theme.isDark);
    EXPECT_EQ(r.theme.editorBg, QColor("#112233"));
    EXPECT_EQ(r.theme.editorFg, QColor("#FFEEDD"));
    EXPECT_EQ(r.descriptor.id.toStdString(), "ocean");
}

TEST(ThemeLoaderTest, T2_ColorFormats) {
    const QString toml = QString::fromUtf8(
        "[editor]\n"
        "background = \"#ABC\"\n"              // #RGB
        "foreground = \"#102030\"\n"           // #RRGGBB
        "current_line = \"#11223380\"\n"       // #RRGGBBAA
        "selection = \"rgb(10, 20, 30)\"\n"    // rgb()
        "line_number = \"rgba(1, 2, 3, 128)\"\n" // rgba 0..255
        "cursor = \"rgba(1, 2, 3, 0.5)\"\n"    // rgba 0..1
    );
    LoadResult r = ThemeLoader::loadFromString(toml);
    ASSERT_TRUE(r.errors.isEmpty()) << r.errors.join(";").toStdString();
    EXPECT_EQ(r.theme.editorBg, QColor("#AABBCC"));
    EXPECT_EQ(r.theme.editorFg, QColor(0x10, 0x20, 0x30));
    EXPECT_EQ(r.theme.editorCurrentLine.alpha(), 0x80);
    EXPECT_EQ(r.theme.editorCurrentLine.red(),   0x11);
    EXPECT_EQ(r.theme.editorSelection, QColor(10, 20, 30));
    EXPECT_EQ(r.theme.editorLineNumber, QColor(1, 2, 3, 128));
    // rgba 0..1 → 0.5 * 255 = 128（rounded）
    EXPECT_NEAR(r.theme.editorCursor.alpha(), 128, 1);
}

TEST(ThemeLoaderTest, T3_IgnoresCommentsAndBlankLines) {
    const QString toml = QString::fromUtf8(
        "# top-level comment\n"
        "\n"
        "[meta]      # inline comment\n"
        "name = \"Foo\" # trailing comment\n"
        "\n"
        "\n"
        "[editor]\n"
        "# another comment\n"
        "background = \"#FFFFFF\"\n"
    );
    LoadResult r = ThemeLoader::loadFromString(toml);
    ASSERT_TRUE(r.errors.isEmpty()) << r.errors.join(";").toStdString();
    EXPECT_EQ(r.theme.name.toStdString(), "Foo");
    EXPECT_EQ(r.theme.editorBg, QColor("#FFFFFF"));
}

TEST(ThemeLoaderTest, T4_InvalidColorRecordedAsError_OthersStillApplied) {
    const QString toml = QString::fromUtf8(
        "[editor]\n"
        "background = \"not-a-color\"\n"
        "foreground = \"#00FF00\"\n"
    );
    LoadResult r = ThemeLoader::loadFromString(toml);
    EXPECT_FALSE(r.errors.isEmpty());
    // 坏字段应保留默认值，好字段应被应用
    EXPECT_EQ(r.theme.editorFg, QColor("#00FF00"));
}

TEST(ThemeLoaderTest, T5_UnknownKeyCollectedAsWarning) {
    const QString toml = QString::fromUtf8(
        "[editor]\n"
        "background = \"#000\"\n"
        "not_a_real_field = \"#FFF\"\n"
    );
    LoadResult r = ThemeLoader::loadFromString(toml);
    ASSERT_TRUE(r.errors.isEmpty()) << r.errors.join(";").toStdString();
    bool found = false;
    for (const QString& w : r.warnings) {
        if (w.contains(QStringLiteral("not_a_real_field"))) { found = true; break; }
    }
    EXPECT_TRUE(found) << "expected warning for unknown key";
}

TEST(ThemeLoaderTest, T6_BuiltinThemesLoadCleanly) {
    for (const char* id : { "light", "dark",
                            "arctic-frost", "sunset-haze", "midnight-aurora",
                            "paper-mist", "mint-breeze", "monochrome-zen" }) {
        const QString path = QStringLiteral(":/themes/") + QString::fromLatin1(id)
                              + QStringLiteral(".toml");
        LoadResult r = ThemeLoader::loadFromFile(path);
        EXPECT_TRUE(r.errors.isEmpty())
            << "builtin " << id << " errors: " << r.errors.join(";").toStdString();
        EXPECT_EQ(r.descriptor.id.toStdString(), std::string(id));
        EXPECT_FALSE(r.theme.name.isEmpty());
    }
}

TEST(ThemeLoaderTest, T7_RoundTripSerializeParse) {
    // 取内置 light 主题 → 序列化 → 再 parse → 检查核心字段相等
    LoadResult r1 = ThemeLoader::loadFromFile(QStringLiteral(":/themes/light.toml"));
    ASSERT_TRUE(r1.errors.isEmpty());

    const QString serialized = ThemeLoader::serialize(r1.theme);
    LoadResult r2 = ThemeLoader::loadFromString(serialized, QStringLiteral("<round-trip>"));
    ASSERT_TRUE(r2.errors.isEmpty()) << r2.errors.join(";").toStdString();

    EXPECT_EQ(r1.theme.editorBg,         r2.theme.editorBg);
    EXPECT_EQ(r1.theme.editorFg,         r2.theme.editorFg);
    EXPECT_EQ(r1.theme.previewBg,        r2.theme.previewBg);
    EXPECT_EQ(r1.theme.previewCodeBg,    r2.theme.previewCodeBg);
    EXPECT_EQ(r1.theme.previewCodeBorder,r2.theme.previewCodeBorder);
}

TEST(ThemeLoaderTest, T8a_ThemeLightNotCrash) {
    Theme a = Theme::light();
    EXPECT_FALSE(a.name.isEmpty());
}

TEST(ThemeLoaderTest, T8b_ThemeDarkNotCrash) {
    Theme a = Theme::dark();
    EXPECT_TRUE(a.isDark);
}

TEST(ThemeLoaderTest, T8c_ThemeByIdLight) {
    Theme b = Theme::byId(QStringLiteral("light"));
    EXPECT_FALSE(b.isDark);
}

TEST(ThemeLoaderTest, T8_ThemeByIdMatchesBuiltin) {
    // Theme::byId("light") 应等同于 Theme::light()，同理 dark。
    // 不能直接比 struct（未定义 ==），抽关键字段比。
    Theme a = Theme::light();
    Theme b = Theme::byId(QStringLiteral("light"));
    EXPECT_EQ(a.name,             b.name);
    EXPECT_EQ(a.isDark,           b.isDark);
    EXPECT_EQ(a.editorBg,         b.editorBg);
    EXPECT_EQ(a.previewBg,        b.previewBg);

    Theme c = Theme::dark();
    Theme d = Theme::byId(QStringLiteral("dark"));
    EXPECT_EQ(c.isDark, d.isDark);
    EXPECT_EQ(c.editorBg, d.editorBg);
}

// GoogleTest 自己有 main；但 Theme::byId 路径需要 QGuiApplication/QCoreApplication
// 存在才能读 palette。FrontmatterRenderTest 已经示范了这点：用 gtest_main +
// 在构造时构造 QCoreApplication。这里也同样 wrapper 一下。
int main(int argc, char** argv) {
    // QGuiApplication 而非 QCoreApplication：Theme::light() 内部会访问
    // QGuiApplication::palette() 的 Highlight 色，没有 QGuiApp 实例时相关
    // 静态方法可能走到未初始化分支。另外 Qt5 下 QtGui 组件在 qrc 资源注册
    // 阶段也可能依赖 QGuiApplication。
    QGuiApplication app(argc, argv);
    Q_INIT_RESOURCE(resources);  // 保证 :/themes/*.toml 可达
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
