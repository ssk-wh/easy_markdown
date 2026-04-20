// src/app/FolderPanel.cpp
//
// Plan: plans/2026-04-14-文件夹侧栏.md
#include "FolderPanel.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTreeView>
#include <QFileSystemModel>
#include <QLabel>
#include <QToolButton>
#include <QMenu>
#include <QContextMenuEvent>
#include <QInputDialog>
#include <QMessageBox>
#include <QDir>
#include <QFileInfo>
#include <QDesktopServices>
#include <QUrl>
#include <QProcess>
#include <QHeaderView>
#include <QCoreApplication>
#include <QStyledItemDelegate>
#include <QPainter>

// 去掉 item 选中时的虚线焦点框
class NoFocusDelegate : public QStyledItemDelegate {
public:
    using QStyledItemDelegate::QStyledItemDelegate;
    void paint(QPainter* painter, const QStyleOptionViewItem& option,
               const QModelIndex& index) const override
    {
        QStyleOptionViewItem opt = option;
        opt.state &= ~QStyle::State_HasFocus;
        QStyledItemDelegate::paint(painter, opt, index);
    }
};

FolderPanel::FolderPanel(QWidget* parent)
    : QWidget(parent)
{
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    // 标题栏
    auto* headerWidget = new QWidget(this);
    auto* headerLayout = new QHBoxLayout(headerWidget);
    headerLayout->setContentsMargins(8, 4, 4, 4);
    headerLayout->setSpacing(4);

    m_titleLabel = new QLabel(tr("Explorer"), headerWidget);
    m_titleLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    QFont titleFont = m_titleLabel->font();
    titleFont.setPointSizeF(titleFont.pointSizeF() * 0.85);
    titleFont.setBold(true);
    m_titleLabel->setFont(titleFont);

    m_closeBtn = new QToolButton(headerWidget);
    m_closeBtn->setText(QStringLiteral("\u00D7"));  // ×
    m_closeBtn->setAutoRaise(true);
    m_closeBtn->setFixedSize(20, 20);
    m_closeBtn->setToolTip(tr("Close Folder"));
    connect(m_closeBtn, &QToolButton::clicked, this, [this]() {
        clearRoot();
    });

    headerLayout->addWidget(m_titleLabel, 1);
    headerLayout->addWidget(m_closeBtn);
    layout->addWidget(headerWidget);

    // 树视图
    m_model = new QFileSystemModel(this);
    m_model->setReadOnly(false);
    m_model->setNameFilters({
        QStringLiteral("*.md"),
        QStringLiteral("*.markdown"),
        QStringLiteral("*.txt")
    });
    m_model->setNameFilterDisables(false);  // 隐藏不匹配的文件（而非灰显）

    m_treeView = new QTreeView(this);
    m_treeView->setModel(m_model);
    m_treeView->setItemDelegate(new NoFocusDelegate(m_treeView));
    m_treeView->setHeaderHidden(true);
    // 只显示 Name 列
    m_treeView->hideColumn(1);  // Size
    m_treeView->hideColumn(2);  // Type
    m_treeView->hideColumn(3);  // Date Modified
    m_treeView->setAnimated(true);
    m_treeView->setIndentation(16);
    m_treeView->setEditTriggers(QTreeView::NoEditTriggers);
    m_treeView->setContextMenuPolicy(Qt::DefaultContextMenu);

    // 单击 → 当前 Tab 打开；双击 → 新 Tab 打开
    connect(m_treeView, &QTreeView::clicked, this, &FolderPanel::onItemClicked);
    connect(m_treeView, &QTreeView::doubleClicked, this, &FolderPanel::onItemDoubleClicked);

    layout->addWidget(m_treeView, 1);

    setMinimumWidth(120);
    hide();  // 默认隐藏，打开文件夹时才显示
}

void FolderPanel::setRootPath(const QString& path)
{
    if (path.isEmpty()) {
        clearRoot();
        return;
    }

    m_rootPath = path;
    QModelIndex rootIndex = m_model->setRootPath(path);
    m_treeView->setRootIndex(rootIndex);

    QDir dir(path);
    m_titleLabel->setText(dir.dirName());
    m_titleLabel->setToolTip(path);

    show();
}

void FolderPanel::selectFile(const QString& filePath)
{
    if (m_rootPath.isEmpty() || !isVisible()) return;

    // 只选中位于当前 rootPath 下的文件
    QString absPath = QFileInfo(filePath).absoluteFilePath();
    if (!absPath.startsWith(m_rootPath)) return;

    QModelIndex index = m_model->index(absPath);
    if (index.isValid()) {
        m_treeView->setCurrentIndex(index);
        m_treeView->scrollTo(index);
    }
}

void FolderPanel::clearRoot()
{
    m_rootPath.clear();
    m_titleLabel->setText(tr("Explorer"));
    m_titleLabel->setToolTip(QString());
    hide();
}

void FolderPanel::setTheme(const Theme& theme)
{
    m_theme = theme;
    applyThemeStyles();
}

void FolderPanel::applyThemeStyles()
{
    // 用主题色设置背景和文字
    QString bg = m_theme.previewBg.name();
    QString fg = m_theme.previewFg.name();
    QString selBg = m_theme.previewLink.name();
    QString headerBg = m_theme.isDark
        ? QColor(m_theme.previewBg.lighter(120)).name()
        : QColor(m_theme.previewBg.darker(105)).name();
    QString borderColor = m_theme.isDark
        ? QColor(m_theme.previewBg.lighter(150)).name()
        : QColor(m_theme.previewBg.darker(115)).name();

    setStyleSheet(QString(
        "FolderPanel { background: %1; }"
        "FolderPanel QLabel { color: %2; }"
        "FolderPanel QToolButton { color: %2; border: none; }"
        "FolderPanel QToolButton:hover { background: %5; }"
    ).arg(bg, fg, selBg, headerBg, borderColor));

    m_treeView->setStyleSheet(QString(
        "QTreeView { background: %1; color: %2; border: none; }"
        "QTreeView::item { padding: 2px 0; }"
        "QTreeView::item:selected { background: %3; color: white; }"
        "QTreeView::item:hover { background: %4; }"
        "QTreeView::branch { background: %1; }"
    ).arg(bg, fg, selBg, borderColor));
}

void FolderPanel::onItemClicked(const QModelIndex& index)
{
    if (!m_model->isDir(index)) {
        emit fileClicked(m_model->filePath(index));
    }
}

void FolderPanel::onItemDoubleClicked(const QModelIndex& index)
{
    if (!m_model->isDir(index)) {
        emit fileDoubleClicked(m_model->filePath(index));
    }
}

void FolderPanel::contextMenuEvent(QContextMenuEvent* event)
{
    QModelIndex index = m_treeView->indexAt(m_treeView->viewport()->mapFrom(this, event->pos()));
    QString path;
    QString parentDir;

    if (index.isValid()) {
        path = m_model->filePath(index);
        parentDir = m_model->isDir(index) ? path : QFileInfo(path).absolutePath();
    } else {
        parentDir = m_rootPath;
    }

    if (parentDir.isEmpty()) return;

    QMenu menu(this);

    menu.addAction(tr("New File..."), [this, parentDir]() { newFile(parentDir); });
    menu.addAction(tr("New Folder..."), [this, parentDir]() { newFolder(parentDir); });

    if (index.isValid()) {
        menu.addSeparator();
        menu.addAction(tr("Rename..."), [this, path]() { renameItem(path); });

        QAction* delAct = menu.addAction(tr("Delete"), [this, path]() { deleteItem(path); });
        // 安全提示色
        delAct->setIcon(QIcon());

        menu.addSeparator();
#ifdef _WIN32
        menu.addAction(tr("Reveal in Explorer"), [this, path]() { revealInExplorer(path); });
#else
        menu.addAction(tr("Reveal in File Manager"), [this, path]() { revealInExplorer(path); });
#endif
    }

    menu.exec(event->globalPos());
}

void FolderPanel::newFile(const QString& parentDir)
{
    bool ok;
    QString name = QInputDialog::getText(this, tr("New File"),
                                          tr("File name:"), QLineEdit::Normal,
                                          QStringLiteral("untitled.md"), &ok);
    if (!ok || name.isEmpty()) return;

    // 确保有 .md 后缀
    if (!name.endsWith(".md") && !name.endsWith(".markdown"))
        name += ".md";

    QString fullPath = QDir(parentDir).filePath(name);
    QFile file(fullPath);
    if (file.exists()) {
        QMessageBox::warning(this, tr("Error"), tr("File already exists: %1").arg(name));
        return;
    }
    if (file.open(QIODevice::WriteOnly)) {
        file.close();
        // 自动打开新创建的文件
        emit fileClicked(fullPath);
    }
}

void FolderPanel::newFolder(const QString& parentDir)
{
    bool ok;
    QString name = QInputDialog::getText(this, tr("New Folder"),
                                          tr("Folder name:"), QLineEdit::Normal,
                                          tr("New Folder"), &ok);
    if (!ok || name.isEmpty()) return;

    QDir dir(parentDir);
    if (!dir.mkdir(name)) {
        QMessageBox::warning(this, tr("Error"), tr("Failed to create folder: %1").arg(name));
    }
}

void FolderPanel::renameItem(const QString& path)
{
    QFileInfo fi(path);
    bool ok;
    QString newName = QInputDialog::getText(this, tr("Rename"),
                                             tr("New name:"), QLineEdit::Normal,
                                             fi.fileName(), &ok);
    if (!ok || newName.isEmpty() || newName == fi.fileName()) return;

    QString newPath = fi.absolutePath() + "/" + newName;
    if (QFile::exists(newPath)) {
        QMessageBox::warning(this, tr("Error"), tr("A file with that name already exists."));
        return;
    }

    if (!QFile::rename(path, newPath)) {
        QMessageBox::warning(this, tr("Error"), tr("Failed to rename."));
    }
}

void FolderPanel::deleteItem(const QString& path)
{
    QFileInfo fi(path);
    QString msg = fi.isDir()
        ? tr("Delete folder \"%1\" and all its contents?").arg(fi.fileName())
        : tr("Delete file \"%1\"?").arg(fi.fileName());

    if (QMessageBox::question(this, tr("Confirm Delete"), msg,
                               QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes) {
        return;
    }

    if (fi.isDir()) {
        QDir(path).removeRecursively();
    } else {
        QFile::remove(path);
    }
}

void FolderPanel::revealInExplorer(const QString& path)
{
#ifdef _WIN32
    QStringList args;
    args << "/select," << QDir::toNativeSeparators(path);
    QProcess::startDetached("explorer.exe", args);
#elif defined(__APPLE__)
    QProcess::startDetached("open", {"-R", path});
#else
    QDesktopServices::openUrl(QUrl::fromLocalFile(QFileInfo(path).absolutePath()));
#endif
}
