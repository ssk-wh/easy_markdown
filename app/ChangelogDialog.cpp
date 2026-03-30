#include "ChangelogDialog.h"

#include <QTextEdit>
#include <QVBoxLayout>
#include <QPushButton>
#include <QFile>
#include <QApplication>
#include <QStandardPaths>
#include <QDebug>

ChangelogDialog::ChangelogDialog(QWidget* parent)
    : QDialog(parent), m_textEdit(nullptr)
{
    setWindowTitle(tr("Update History"));
    setMinimumSize(700, 500);

    // 文本编辑框
    m_textEdit = new QTextEdit(this);
    m_textEdit->setReadOnly(true);

    // 关闭按钮
    QPushButton* closeBtn = new QPushButton(tr("Close"), this);
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(m_textEdit);
    layout->addWidget(closeBtn);

    loadChangelog();
}

void ChangelogDialog::loadChangelog()
{
    // 搜索优先级：应用目录 > 相对开发目录 > 系统安装路径
    QStringList paths;

    // Windows / Linux：应用同目录
    paths << QApplication::applicationDirPath() + "/CHANGELOG.md";

    // 开发环境：项目根目录（从 app/ 目录算起）
    paths << QApplication::applicationDirPath() + "/../CHANGELOG.md";
    paths << QApplication::applicationDirPath() + "/../../CHANGELOG.md";

    // Linux：系统路径
#ifdef Q_OS_LINUX
    paths << "/usr/share/doc/simplemarkdown/CHANGELOG.md";
    paths << "/usr/local/share/doc/simplemarkdown/CHANGELOG.md";
#endif

    QString content;
    for (const auto& path : paths) {
        QFile file(path);
        if (file.exists() && file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            content = QString::fromUtf8(file.readAll());
            qDebug() << "[ChangelogDialog] Loaded from:" << path;
            break;
        }
    }

    if (content.isEmpty()) {
        content = tr("Update history file not found.\n\n"
                     "Please ensure CHANGELOG.md is installed with the application.");
    }

    // 渲染 Markdown 格式
    // 简单的 Markdown 格式渲染（标题、列表、代码块）
    QString html = "<html><body style=\"font-family: monospace; white-space: pre-wrap;\">";

    // 基础 Markdown 渲染
    QStringList lines = content.split('\n');
    for (const auto& line : lines) {
        QString htmlLine = line;

        // 转义 HTML 特殊字符
        htmlLine.replace("&", "&amp;")
                .replace("<", "&lt;")
                .replace(">", "&gt;");

        // Markdown 标题处理（# ## ###）
        if (htmlLine.startsWith("# ")) {
            htmlLine.replace(0, 2, "");
            htmlLine = QString("<h2 style=\"margin-top: 16px; margin-bottom: 8px; font-weight: bold;\">%1</h2>").arg(htmlLine);
        } else if (htmlLine.startsWith("## ")) {
            htmlLine.replace(0, 3, "");
            htmlLine = QString("<h3 style=\"margin-top: 12px; margin-bottom: 6px; font-weight: bold;\">%1</h3>").arg(htmlLine);
        } else if (htmlLine.startsWith("### ")) {
            htmlLine.replace(0, 4, "");
            htmlLine = QString("<h4 style=\"margin-top: 10px; margin-bottom: 4px; font-weight: bold;\">%1</h4>").arg(htmlLine);
        }
        // 列表处理
        else if (htmlLine.startsWith("- ")) {
            htmlLine.replace(0, 2, "");
            htmlLine = QString("<div style=\"margin-left: 20px;\">• %1</div>").arg(htmlLine);
        }

        html += htmlLine + "\n";
    }

    html += "</body></html>";

    m_textEdit->setHtml(html);
    m_textEdit->moveCursor(QTextCursor::Start);  // 回到顶部
    m_textEdit->ensureCursorVisible();
}
