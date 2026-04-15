// Spec: specs/模块-app/12-主题插件系统.md
// Spec: specs/横切关注点/30-主题系统.md (INV-6)
// Last synced: 2026-04-14
#include "ThemeLoader.h"
#include "ThemeSchema.h"

#include <QByteArray>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QHash>
#include <QStandardPaths>
#include <QStringList>
#include <QTextStream>
#include <QVariant>

namespace core {

namespace {

// ---------------- 极简 TOML 子集解析器 ----------------
//
// 将 TOML 转成一棵扁平的 QHash<QString, QVariant>：
//   [editor]
//   background = "#fff"
// 变成 { "editor.background": "#fff" }
//
// 错误收集到 errors，不打断解析。

struct ParsedToml {
    QHash<QString, QVariant> flat;
    QStringList errors;
    QStringList warnings;
};

static QString trimmed(const QString& s) { return s.trimmed(); }

// 去掉行尾注释（# 之后的内容），但不能破坏字符串字面量中的 #
static QString stripLineComment(const QString& line) {
    bool inString = false;
    QChar quote;
    for (int i = 0; i < line.size(); ++i) {
        const QChar c = line[i];
        if (inString) {
            if (c == QLatin1Char('\\') && i + 1 < line.size()) {
                ++i; // 跳过转义
                continue;
            }
            if (c == quote) inString = false;
        } else {
            if (c == QLatin1Char('"') || c == QLatin1Char('\'')) {
                inString = true;
                quote = c;
            } else if (c == QLatin1Char('#')) {
                return line.left(i);
            }
        }
    }
    return line;
}

// 去字符串字面量两侧引号；支持基本转义 \" \\ \n \t
static bool parseStringLiteral(const QString& raw, QString* out, QString* err) {
    if (raw.size() < 2) return false;
    const QChar q = raw.front();
    if ((q != QLatin1Char('"') && q != QLatin1Char('\''))
        || raw.back() != q) {
        if (err) *err = QStringLiteral("unterminated string: %1").arg(raw);
        return false;
    }
    QString body = raw.mid(1, raw.size() - 2);
    QString outStr;
    outStr.reserve(body.size());
    for (int i = 0; i < body.size(); ++i) {
        QChar c = body[i];
        if (c == QLatin1Char('\\') && i + 1 < body.size()) {
            QChar n = body[i + 1];
            switch (n.toLatin1()) {
                case 'n': outStr += QLatin1Char('\n'); break;
                case 't': outStr += QLatin1Char('\t'); break;
                case 'r': outStr += QLatin1Char('\r'); break;
                case '"': outStr += QLatin1Char('"');  break;
                case '\'': outStr += QLatin1Char('\''); break;
                case '\\': outStr += QLatin1Char('\\'); break;
                default:
                    // 未知转义按原样保留
                    outStr += c;
                    outStr += n;
                    break;
            }
            ++i;
        } else {
            outStr += c;
        }
    }
    *out = outStr;
    return true;
}

static QVariant parseValue(const QString& raw, QString* err) {
    const QString v = raw.trimmed();
    if (v.isEmpty()) {
        if (err) *err = QStringLiteral("empty value");
        return QVariant();
    }
    if (v.startsWith(QLatin1Char('"')) || v.startsWith(QLatin1Char('\''))) {
        QString s;
        if (parseStringLiteral(v, &s, err)) return s;
        return QVariant();
    }
    if (v == QLatin1String("true"))  return true;
    if (v == QLatin1String("false")) return false;

    // 数字（int 或 float）
    bool okInt = false;
    qlonglong i = v.toLongLong(&okInt);
    if (okInt) return QVariant(i);
    bool okDouble = false;
    double d = v.toDouble(&okDouble);
    if (okDouble) return QVariant(d);

    if (err) *err = QStringLiteral("unrecognized value: %1").arg(v);
    return QVariant();
}

static ParsedToml parseToml(const QString& raw, const QString& source) {
    ParsedToml out;
    QString currentSection; // 空表示根
    int lineno = 0;
    for (const QString& rawLine : raw.split(QLatin1Char('\n'))) {
        ++lineno;
        QString line = stripLineComment(rawLine).trimmed();
        if (line.isEmpty()) continue;

        // [section] 或 [section.subsection]
        if (line.startsWith(QLatin1Char('[')) && line.endsWith(QLatin1Char(']'))) {
            QString name = line.mid(1, line.size() - 2).trimmed();
            if (name.isEmpty()) {
                out.errors << QStringLiteral("%1:%2: empty section header")
                                .arg(source).arg(lineno);
                continue;
            }
            currentSection = name;
            continue;
        }

        // key = value
        int eq = -1;
        // 定位第一个不在字符串字面量内的 =
        {
            bool inStr = false; QChar q;
            for (int i = 0; i < line.size(); ++i) {
                const QChar c = line[i];
                if (inStr) {
                    if (c == QLatin1Char('\\')) { ++i; continue; }
                    if (c == q) inStr = false;
                } else {
                    if (c == QLatin1Char('"') || c == QLatin1Char('\'')) {
                        inStr = true; q = c;
                    } else if (c == QLatin1Char('=')) {
                        eq = i; break;
                    }
                }
            }
        }
        if (eq < 0) {
            out.errors << QStringLiteral("%1:%2: expected 'key = value' or '[section]', got: %3")
                            .arg(source).arg(lineno).arg(line);
            continue;
        }
        QString key = line.left(eq).trimmed();
        QString val = line.mid(eq + 1).trimmed();
        if (key.isEmpty()) {
            out.errors << QStringLiteral("%1:%2: empty key").arg(source).arg(lineno);
            continue;
        }
        QString err;
        QVariant v = parseValue(val, &err);
        if (!v.isValid()) {
            out.errors << QStringLiteral("%1:%2: %3").arg(source).arg(lineno).arg(err);
            continue;
        }
        const QString fullKey = currentSection.isEmpty()
            ? key
            : currentSection + QLatin1Char('.') + key;
        if (out.flat.contains(fullKey)) {
            out.warnings << QStringLiteral("%1:%2: duplicate key '%3' (last value wins)")
                                .arg(source).arg(lineno).arg(fullKey);
        }
        out.flat.insert(fullKey, v);
    }
    return out;
}

// ---------------- 色值解析 ----------------
//
// 支持:
//   - #RGB       → #RRGGBB
//   - #RRGGBB
//   - #RRGGBBAA  (Qt 原生支持 #AARRGGBB，所以这里做转换)
//   - rgb(r,g,b)
//   - rgba(r,g,b,a) 其中 a ∈ [0,1] 或 0..255
//
// 返回 invalid QColor 代表失败。

static QColor parseColor(const QString& raw, QString* err) {
    QString s = raw.trimmed();
    if (s.isEmpty()) { if (err) *err = QStringLiteral("empty color"); return QColor(); }

    if (s.startsWith(QLatin1Char('#'))) {
        const QString hex = s.mid(1);
        if (hex.size() == 3) {
            // #RGB
            QString expand;
            for (QChar c : hex) { expand += c; expand += c; }
            QColor c(QStringLiteral("#") + expand);
            if (!c.isValid() && err) *err = QStringLiteral("invalid hex: %1").arg(raw);
            return c;
        }
        if (hex.size() == 6) {
            QColor c(s);
            if (!c.isValid() && err) *err = QStringLiteral("invalid hex: %1").arg(raw);
            return c;
        }
        if (hex.size() == 8) {
            // #RRGGBBAA → Qt 需要 #AARRGGBB
            QString rr = hex.mid(0, 2);
            QString gg = hex.mid(2, 2);
            QString bb = hex.mid(4, 2);
            QString aa = hex.mid(6, 2);
            QColor c(QStringLiteral("#") + aa + rr + gg + bb);
            if (!c.isValid() && err) *err = QStringLiteral("invalid hex: %1").arg(raw);
            return c;
        }
        if (err) *err = QStringLiteral("hex color must be #RGB/#RRGGBB/#RRGGBBAA: %1").arg(raw);
        return QColor();
    }

    if (s.startsWith(QLatin1String("rgb(")) && s.endsWith(QLatin1Char(')'))) {
        QString body = s.mid(4, s.size() - 5);
        QStringList parts = body.split(QLatin1Char(','), QString::SkipEmptyParts);
        if (parts.size() == 3) {
            int r = parts[0].trimmed().toInt();
            int g = parts[1].trimmed().toInt();
            int b = parts[2].trimmed().toInt();
            return QColor(r, g, b);
        }
    }
    if (s.startsWith(QLatin1String("rgba(")) && s.endsWith(QLatin1Char(')'))) {
        QString body = s.mid(5, s.size() - 6);
        QStringList parts = body.split(QLatin1Char(','), QString::SkipEmptyParts);
        if (parts.size() == 4) {
            int r = parts[0].trimmed().toInt();
            int g = parts[1].trimmed().toInt();
            int b = parts[2].trimmed().toInt();
            double af = parts[3].trimmed().toDouble();
            int a = (af <= 1.0) ? int(af * 255.0 + 0.5) : int(af);
            return QColor(r, g, b, a);
        }
    }

    if (err) *err = QStringLiteral("unrecognized color format: %1").arg(raw);
    return QColor();
}

// ---------------- 字段映射 ----------------
//
// 把 flat TOML 表映射到 Theme 的具体字段上。
// 未知字段收集为 warning。

struct ColorTarget {
    const char* tomlKey;
    QColor Theme::* member;
};

static const ColorTarget kColorTargets[] = {
    // editor.*
    { "editor.background",           &Theme::editorBg },
    { "editor.foreground",           &Theme::editorFg },
    { "editor.current_line",         &Theme::editorCurrentLine },
    { "editor.selection",            &Theme::editorSelection },
    { "editor.line_number",          &Theme::editorLineNumber },
    { "editor.line_number_active",   &Theme::editorLineNumberActive },
    { "editor.gutter_background",    &Theme::editorGutterBg },
    { "editor.gutter_line",          &Theme::editorGutterLine },
    { "editor.cursor",               &Theme::editorCursor },
    { "editor.search_match",         &Theme::editorSearchMatch },
    { "editor.search_match_current", &Theme::editorSearchMatchCurrent },
    { "editor.preedit_background",   &Theme::editorPreeditBg },

    // syntax.*
    { "syntax.heading",              &Theme::syntaxHeading },
    { "syntax.code",                 &Theme::syntaxCode },
    { "syntax.code_background",      &Theme::syntaxCodeBg },
    { "syntax.code_block",           &Theme::syntaxCodeBlock },
    { "syntax.code_block_background",&Theme::syntaxCodeBlockBg },
    { "syntax.link",                 &Theme::syntaxLink },
    { "syntax.list",                 &Theme::syntaxList },
    { "syntax.blockquote",           &Theme::syntaxBlockQuote },
    { "syntax.fence",                &Theme::syntaxFence },
    { "syntax.fence_background",     &Theme::syntaxFenceBg },

    // preview.*
    { "preview.background",             &Theme::previewBg },
    { "preview.foreground",             &Theme::previewFg },
    { "preview.heading",                &Theme::previewHeading },
    { "preview.link",                   &Theme::previewLink },
    { "preview.code_background",        &Theme::previewCodeBg },
    { "preview.code_foreground",        &Theme::previewCodeFg },
    { "preview.code_border",            &Theme::previewCodeBorder },
    // 行内代码 / 代码块拆分字段（可选，不写时 fallback 到 preview.code_*）
    { "preview.inline_code_background", &Theme::previewInlineCodeBg },
    { "preview.inline_code_foreground", &Theme::previewInlineCodeFg },
    { "preview.code_block_background",  &Theme::previewCodeBlockBg },
    { "preview.code_block_foreground",  &Theme::previewCodeBlockFg },
    { "preview.code_block_border",      &Theme::previewCodeBlockBorder },
    { "preview.blockquote_border",      &Theme::previewBlockQuoteBorder },
    { "preview.blockquote_background",  &Theme::previewBlockQuoteBg },
    { "preview.table_border",           &Theme::previewTableBorder },
    { "preview.table_header_background",&Theme::previewTableHeaderBg },
    { "preview.hr",                     &Theme::previewHr },
    { "preview.heading_separator",      &Theme::previewHeadingSeparator },
    { "preview.image_placeholder_background", &Theme::previewImagePlaceholderBg },
    { "preview.image_placeholder_border",     &Theme::previewImagePlaceholderBorder },
    { "preview.image_placeholder_text",       &Theme::previewImagePlaceholderText },
    { "preview.image_error_background", &Theme::previewImageErrorBg },
    { "preview.image_error_border",     &Theme::previewImageErrorBorder },
    { "preview.image_error_text",       &Theme::previewImageErrorText },
    { "preview.image_info_text",        &Theme::previewImageInfoText },
    { "preview.highlight",              &Theme::previewHighlight },
    { "preview.highlight_toc",          &Theme::previewHighlightToc },
    { "preview.accent",                 &Theme::accentColor },
    { "preview.frontmatter_background", &Theme::frontmatterBackground },
    { "preview.frontmatter_border",     &Theme::frontmatterBorder },
    { "preview.frontmatter_key",        &Theme::frontmatterKeyForeground },
    { "preview.frontmatter_value",      &Theme::frontmatterValueForeground },
};

static bool isKnownScalarKey(const QString& key) {
    static const QStringList known = {
        QStringLiteral("meta.name"),
        QStringLiteral("meta.id"),
        QStringLiteral("meta.is_dark"),
        QStringLiteral("meta.version"),
        QStringLiteral("meta.author"),
        QStringLiteral("meta.description"),
        QStringLiteral("meta.market_url"),
        QStringLiteral("meta.extends"),
    };
    return known.contains(key);
}

static void applyToTheme(const ParsedToml& p, LoadResult& r) {
    // meta
    if (p.flat.contains(QStringLiteral("meta.name")))
        r.theme.name = p.flat.value(QStringLiteral("meta.name")).toString();
    if (p.flat.contains(QStringLiteral("meta.is_dark")))
        r.theme.isDark = p.flat.value(QStringLiteral("meta.is_dark")).toBool();
    if (p.flat.contains(QStringLiteral("meta.id")))
        r.descriptor.id = p.flat.value(QStringLiteral("meta.id")).toString();
    if (p.flat.contains(QStringLiteral("meta.name")))
        r.descriptor.displayName = r.theme.name;

    // 色值字段
    QHash<QString, bool> knownColorKeys;
    for (const auto& t : kColorTargets) knownColorKeys.insert(QString::fromLatin1(t.tomlKey), true);

    for (const auto& t : kColorTargets) {
        const QString key = QString::fromLatin1(t.tomlKey);
        if (!p.flat.contains(key)) continue;
        const QString raw = p.flat.value(key).toString();
        QString err;
        QColor c = parseColor(raw, &err);
        if (!c.isValid()) {
            r.errors << QStringLiteral("field '%1': %2").arg(key).arg(err);
            continue;
        }
        r.theme.*(t.member) = c;
    }

    // 扫未知字段 → warning
    for (auto it = p.flat.cbegin(); it != p.flat.cend(); ++it) {
        const QString& k = it.key();
        if (knownColorKeys.contains(k)) continue;
        if (isKnownScalarKey(k)) continue;
        r.warnings << QStringLiteral("unknown field '%1' (ignored)").arg(k);
    }
}

static QString stripUtf8Bom(QByteArray& bytes) {
    if (bytes.size() >= 3
        && static_cast<unsigned char>(bytes[0]) == 0xEF
        && static_cast<unsigned char>(bytes[1]) == 0xBB
        && static_cast<unsigned char>(bytes[2]) == 0xBF) {
        bytes.remove(0, 3);
    }
    return QString::fromUtf8(bytes);
}

} // namespace

LoadResult ThemeLoader::loadFromString(const QString& toml, const QString& source) {
    LoadResult r;
    ThemeDescriptor desc;
    desc.filePath = source;
    r.descriptor = desc;

    ParsedToml parsed = parseToml(toml, source);
    r.errors    = parsed.errors;
    r.warnings  = parsed.warnings;

    applyToTheme(parsed, r);

    // descriptor.id fallback: 文件名
    if (r.descriptor.id.isEmpty()) {
        QFileInfo fi(source);
        r.descriptor.id = fi.completeBaseName();
    }
    if (r.descriptor.displayName.isEmpty())
        r.descriptor.displayName = r.descriptor.id;
    r.descriptor.isDark = r.theme.isDark;

    r.ok = r.errors.isEmpty();
    return r;
}

LoadResult ThemeLoader::loadFromFile(const QString& path) {
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) {
        LoadResult r;
        r.errors << QStringLiteral("cannot open '%1': %2").arg(path).arg(f.errorString());
        r.descriptor.filePath = path;
        return r;
    }
    QByteArray bytes = f.readAll();
    f.close();
    QString text = stripUtf8Bom(bytes);
    LoadResult r = loadFromString(text, path);
    r.descriptor.filePath = path;
    r.descriptor.isBuiltin = path.startsWith(QLatin1String(":/"));
    if (r.descriptor.id.isEmpty()) {
        QFileInfo fi(path);
        r.descriptor.id = fi.completeBaseName();
    }
    return r;
}

QList<ThemeDescriptor> ThemeLoader::scanDirectory(const QString& dirPath, bool markBuiltin) {
    QList<ThemeDescriptor> out;
    QDir dir(dirPath);
    if (!dir.exists()) return out;
    const QStringList files = dir.entryList(QStringList() << QStringLiteral("*.toml"),
                                            QDir::Files | QDir::Readable, QDir::Name);
    for (const QString& f : files) {
        // Spec: specs/模块-app/12-主题插件系统.md INV-16
        // "template.toml" 是我们为"目录为空引导自定义"注入的样板，
        // 它本身合法可加载，但不应作为一个可选主题出现在菜单里。
        // 用户必须复制它并改名（例如 my-theme.toml）才会被识别。
        if (QString::compare(f, QStringLiteral("template.toml"), Qt::CaseInsensitive) == 0) {
            continue;
        }
        ThemeDescriptor d;
        const QString full = dir.absoluteFilePath(f);
        d.filePath = full;
        d.isBuiltin = markBuiltin;
        // 只做一次轻量解析来取 meta.name / meta.id / meta.is_dark，不填充 Theme
        QFile file(full);
        if (file.open(QIODevice::ReadOnly)) {
            QByteArray bytes = file.readAll();
            file.close();
            QString text = stripUtf8Bom(bytes);
            ParsedToml p = parseToml(text, full);
            if (p.flat.contains(QStringLiteral("meta.id")))
                d.id = p.flat.value(QStringLiteral("meta.id")).toString();
            if (p.flat.contains(QStringLiteral("meta.name")))
                d.displayName = p.flat.value(QStringLiteral("meta.name")).toString();
            if (p.flat.contains(QStringLiteral("meta.is_dark")))
                d.isDark = p.flat.value(QStringLiteral("meta.is_dark")).toBool();
        }
        if (d.id.isEmpty()) {
            QFileInfo fi(full);
            d.id = fi.completeBaseName();
        }
        if (d.displayName.isEmpty())
            d.displayName = d.id;
        out.append(d);
    }
    return out;
}

QString ThemeLoader::userThemeDir() {
    const QString base = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir d(base);
    if (!d.exists()) d.mkpath(QStringLiteral("."));
    const QString themes = d.absoluteFilePath(QStringLiteral("themes"));
    QDir t(themes);
    if (!t.exists()) t.mkpath(QStringLiteral("."));
    return themes;
}

// Spec: specs/模块-app/12-主题插件系统.md INV-15
bool ThemeLoader::injectThemeTemplatesIfEmpty(const QString& dir,
                                              QStringList* writtenOut,
                                              QStringList* failedOut) {
    if (writtenOut) writtenOut->clear();
    if (failedOut) failedOut->clear();

    QDir d(dir);
    if (!d.exists()) {
        // 目录不存在视为"空"，先创建
        if (!d.mkpath(QStringLiteral("."))) {
            if (failedOut) {
                failedOut->append(QStringLiteral("template.toml"));
                failedOut->append(QStringLiteral("HOW_TO.md"));
            }
            return false;
        }
    }

    // 任何文件存在即视为"非空"，跳过注入（幂等：用户清空后再次打开仍会注入）
    const QStringList files = d.entryList(QDir::Files | QDir::NoDotAndDotDot);
    if (!files.isEmpty()) {
        return false;
    }

    struct DocFile {
        const char* resPath;
        const char* destName;
    };
    static const DocFile kDocs[] = {
        { ":/theme_docs/template.toml", "template.toml" },
        { ":/theme_docs/HOW_TO.md",     "HOW_TO.md" },
    };

    bool anyWritten = false;
    for (const auto& doc : kDocs) {
        QFile src(QString::fromLatin1(doc.resPath));
        if (!src.open(QIODevice::ReadOnly)) {
            if (failedOut) failedOut->append(QString::fromLatin1(doc.destName));
            continue;
        }
        const QByteArray bytes = src.readAll();
        src.close();

        const QString destPath = d.absoluteFilePath(QString::fromLatin1(doc.destName));
        QFile dst(destPath);
        if (!dst.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
            if (failedOut) failedOut->append(QString::fromLatin1(doc.destName));
            continue;
        }
        const qint64 written = dst.write(bytes);
        dst.close();
        if (written != bytes.size()) {
            // 写入不全，删除残文件
            QFile::remove(destPath);
            if (failedOut) failedOut->append(QString::fromLatin1(doc.destName));
            continue;
        }
        if (writtenOut) writtenOut->append(QString::fromLatin1(doc.destName));
        anyWritten = true;
    }
    return anyWritten;
}

QList<ThemeDescriptor> ThemeLoader::discoverAll(const QString& userDirPath) {
    QList<ThemeDescriptor> out;
    // 内置主题列表（硬编码：避免扫 qrc）
    // Spec: specs/模块-app/12-主题插件系统.md
    static const char* kBuiltinIds[] = {
        "light", "dark",
        // iOS 26 Liquid Glass 组
        "arctic-frost", "sunset-haze", "midnight-aurora",
        // 清新 / 极简组
        "paper-mist", "mint-breeze", "monochrome-zen"
    };
    for (const char* id : kBuiltinIds) {
        const QString path = QStringLiteral(":/themes/") + QString::fromLatin1(id) + QStringLiteral(".toml");
        LoadResult r = loadFromFile(path);
        if (r.ok) {
            ThemeDescriptor d = r.descriptor;
            d.isBuiltin = true;
            if (d.id.isEmpty()) d.id = QString::fromLatin1(id);
            out.append(d);
        }
    }
    // 用户目录
    const QList<ThemeDescriptor> userThemes = scanDirectory(userDirPath, /*markBuiltin=*/false);
    QHash<QString, bool> seen;
    for (const auto& d : out) seen.insert(d.id, true);
    for (const auto& d : userThemes) {
        if (seen.contains(d.id)) continue;
        out.append(d);
        seen.insert(d.id, true);
    }
    return out;
}

QString ThemeLoader::serialize(const Theme& theme) {
    QString out;
    QTextStream ts(&out);
    ts << "# Generated by SimpleMarkdown ThemeLoader\n";
    ts << "[meta]\n";
    ts << "name = \"" << theme.name << "\"\n";
    ts << "is_dark = " << (theme.isDark ? "true" : "false") << "\n";
    ts << "version = \"" << theme_schema::kSchemaVersionMajor << "."
       << theme_schema::kSchemaVersionMinor << "\"\n\n";

    // alpha==255 用 #RRGGBB；带 alpha 用 rgba() 避免 Qt 的 HexArgb (#AARRGGBB) 与
    // CSS 的 #RRGGBBAA 位序冲突。
    auto dumpColor = [&](const char* section, const char* key, const QColor& c) {
        if (c.alpha() == 255) {
            ts << key << " = \"" << c.name(QColor::HexRgb).toUpper() << "\"\n";
        } else {
            ts << key << " = \"rgba("
               << c.red() << ", " << c.green() << ", " << c.blue() << ", " << c.alpha()
               << ")\"\n";
        }
        Q_UNUSED(section);
    };
    QString lastSection;
    for (const auto& t : kColorTargets) {
        const QString full = QString::fromLatin1(t.tomlKey);
        const int dot = full.indexOf(QLatin1Char('.'));
        const QString sec = full.left(dot);
        const QString key = full.mid(dot + 1);
        if (sec != lastSection) {
            ts << "\n[" << sec << "]\n";
            lastSection = sec;
        }
        dumpColor(sec.toLatin1().constData(),
                  key.toLatin1().constData(),
                  theme.*(t.member));
    }
    ts.flush();
    return out;
}

} // namespace core
