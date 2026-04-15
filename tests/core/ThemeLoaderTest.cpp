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

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
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

// Spec: specs/模块-app/12-主题插件系统.md INV-15 / T-13 / T-14 / T-15
// 主题目录注入引导文件（template.toml + HOW_TO.md）的行为契约
namespace {

// 用临时目录构造测试 fixture，避免污染真实用户目录
QString makeTempThemeDir(const QString& tag) {
    QDir base(QDir::tempPath());
    const QString name = QStringLiteral("simple_markdown_test_%1_%2")
                             .arg(tag)
                             .arg(QDateTime::currentMSecsSinceEpoch());
    base.mkpath(name);
    return base.absoluteFilePath(name);
}

void rmDirContents(const QString& dir) {
    QDir d(dir);
    if (!d.exists()) return;
    for (const QFileInfo& fi : d.entryInfoList(QDir::Files | QDir::NoDotAndDotDot)) {
        QFile::remove(fi.absoluteFilePath());
    }
}

} // namespace

TEST(ThemeLoaderTest, T13_InjectTemplatesIntoEmptyDir) {
    const QString dir = makeTempThemeDir(QStringLiteral("t13"));
    rmDirContents(dir);  // 确保空

    QStringList written, failed;
    const bool any = ThemeLoader::injectThemeTemplatesIfEmpty(dir, &written, &failed);

    EXPECT_TRUE(any);
    EXPECT_TRUE(failed.isEmpty()) << failed.join(",").toStdString();

    // 必须同时写入两份文件
    ASSERT_EQ(written.size(), 2);
    EXPECT_TRUE(written.contains(QStringLiteral("template.toml")));
    EXPECT_TRUE(written.contains(QStringLiteral("HOW_TO.md")));

    // 文件确实落盘且非空
    QDir d(dir);
    QFile templ(d.absoluteFilePath(QStringLiteral("template.toml")));
    QFile howto(d.absoluteFilePath(QStringLiteral("HOW_TO.md")));
    ASSERT_TRUE(templ.exists());
    ASSERT_TRUE(howto.exists());
    EXPECT_GT(templ.size(), 100);  // 模板内容至少 100 字节
    EXPECT_GT(howto.size(), 100);

    // 清理
    rmDirContents(dir);
    QDir().rmdir(dir);
}

TEST(ThemeLoaderTest, T14_SkipInjectWhenDirNotEmpty) {
    const QString dir = makeTempThemeDir(QStringLiteral("t14"));
    rmDirContents(dir);

    // 放一个用户已有的"假主题"
    QDir d(dir);
    const QString userPath = d.absoluteFilePath(QStringLiteral("user_existing.toml"));
    {
        QFile f(userPath);
        ASSERT_TRUE(f.open(QIODevice::WriteOnly));
        f.write("# user theme placeholder\n");
        f.close();
    }

    QStringList written, failed;
    const bool any = ThemeLoader::injectThemeTemplatesIfEmpty(dir, &written, &failed);

    EXPECT_FALSE(any);
    EXPECT_TRUE(written.isEmpty());
    EXPECT_TRUE(failed.isEmpty());

    // 用户文件还在
    EXPECT_TRUE(QFile::exists(userPath));
    // 模板文件未被注入
    EXPECT_FALSE(QFile::exists(d.absoluteFilePath(QStringLiteral("template.toml"))));
    EXPECT_FALSE(QFile::exists(d.absoluteFilePath(QStringLiteral("HOW_TO.md"))));

    // 清理
    rmDirContents(dir);
    QDir().rmdir(dir);
}

TEST(ThemeLoaderTest, T15_InjectedTemplateIsLoadable) {
    // 注入的 template.toml 必须本身就是合法 TOML 主题，能被 ThemeLoader 加载
    const QString dir = makeTempThemeDir(QStringLiteral("t15"));
    rmDirContents(dir);

    QStringList written, failed;
    ASSERT_TRUE(ThemeLoader::injectThemeTemplatesIfEmpty(dir, &written, &failed));
    ASSERT_EQ(failed.size(), 0);

    QDir d(dir);
    const QString templPath = d.absoluteFilePath(QStringLiteral("template.toml"));
    LoadResult r = ThemeLoader::loadFromFile(templPath);
    EXPECT_TRUE(r.errors.isEmpty()) << r.errors.join(";").toStdString();
    EXPECT_FALSE(r.theme.name.isEmpty());

    rmDirContents(dir);
    QDir().rmdir(dir);
}

TEST(ThemeLoaderTest, T15b_InjectIsIdempotentAfterUserClears) {
    // 用户清空目录后再次调用 → 应再次注入（幂等性）
    const QString dir = makeTempThemeDir(QStringLiteral("t15b"));
    rmDirContents(dir);

    QStringList w1, f1;
    ASSERT_TRUE(ThemeLoader::injectThemeTemplatesIfEmpty(dir, &w1, &f1));
    EXPECT_EQ(w1.size(), 2);

    // 第二次：目录非空 → 不动
    QStringList w2, f2;
    EXPECT_FALSE(ThemeLoader::injectThemeTemplatesIfEmpty(dir, &w2, &f2));
    EXPECT_EQ(w2.size(), 0);

    // 用户清空
    rmDirContents(dir);

    // 第三次：目录又空 → 再注入
    QStringList w3, f3;
    ASSERT_TRUE(ThemeLoader::injectThemeTemplatesIfEmpty(dir, &w3, &f3));
    EXPECT_EQ(w3.size(), 2);

    rmDirContents(dir);
    QDir().rmdir(dir);
}

// Spec: specs/模块-app/12-主题插件系统.md INV-16 / T-17
// 扫描目录时必须跳过 template.toml，避免它被当作一个可选主题出现在菜单中。
TEST(ThemeLoaderTest, T17_ScanDirectorySkipsTemplateToml) {
    const QString dir = makeTempThemeDir(QStringLiteral("t17"));
    rmDirContents(dir);

    // 注入模板
    QStringList written, failed;
    ASSERT_TRUE(ThemeLoader::injectThemeTemplatesIfEmpty(dir, &written, &failed));
    ASSERT_EQ(failed.size(), 0);

    // 再放一份真实的用户主题
    QDir d(dir);
    const QString myPath = d.absoluteFilePath(QStringLiteral("my-theme.toml"));
    {
        QFile f(myPath);
        ASSERT_TRUE(f.open(QIODevice::WriteOnly));
        f.write(
            "[meta]\n"
            "name = \"My Real Theme\"\n"
            "id = \"my-theme\"\n"
            "is_dark = false\n"
            "version = \"1.0\"\n"
            "\n"
            "[editor]\n"
            "background = \"#FFFFFF\"\n"
        );
        f.close();
    }

    // scanDirectory 应跳过 template.toml，只返回 my-theme
    const QList<core::ThemeDescriptor> found = ThemeLoader::scanDirectory(dir, /*markBuiltin=*/false);

    bool sawTemplate = false;
    bool sawMyTheme = false;
    for (const core::ThemeDescriptor& x : found) {
        QFileInfo fi(x.filePath);
        if (QString::compare(fi.fileName(), QStringLiteral("template.toml"), Qt::CaseInsensitive) == 0) {
            sawTemplate = true;
        }
        if (x.id == QStringLiteral("my-theme")) {
            sawMyTheme = true;
        }
    }
    EXPECT_FALSE(sawTemplate) << "template.toml should NOT be listed as a scannable theme";
    EXPECT_TRUE(sawMyTheme) << "user's my-theme.toml should still be discovered";

    // discoverAll 同样不应返回模板
    const QList<core::ThemeDescriptor> all = ThemeLoader::discoverAll(dir);
    for (const core::ThemeDescriptor& x : all) {
        QFileInfo fi(x.filePath);
        EXPECT_NE(
            QString::compare(fi.fileName(), QStringLiteral("template.toml"), Qt::CaseInsensitive),
            0
        ) << "discoverAll() returned template.toml — should be skipped";
    }

    rmDirContents(dir);
    QDir().rmdir(dir);
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
