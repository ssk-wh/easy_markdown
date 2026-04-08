#include "ChangelogDialog.h"
#include "PreviewWidget.h"
#include "MarkdownParser.h"

#include <QVBoxLayout>
#include <QPushButton>
#include <QFile>
#include <QApplication>
#include <QDebug>

ChangelogDialog::ChangelogDialog(const Theme& theme, QWidget* parent)
    : QDialog(parent), m_previewWidget(nullptr)
{
    setWindowTitle(tr("Update History"));
    setMinimumSize(700, 500);

    // 预览组件
    m_previewWidget = new PreviewWidget(this);
    m_previewWidget->setTheme(theme);

    // 关闭按钮
    QPushButton* closeBtn = new QPushButton(tr("Close"), this);
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(m_previewWidget);
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

    // 使用项目的 Markdown 解析器渲染
    MarkdownParser parser;
    auto ast = parser.parse(content);
    m_previewWidget->updateAst(std::shared_ptr<AstNode>(std::move(ast)));
}
