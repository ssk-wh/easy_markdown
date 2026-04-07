#pragma once
#include <QMainWindow>
#include <QTabWidget>
#include <QVector>
#include <QTimer>
#include <QFileSystemWatcher>
#include "Theme.h"

class EditorWidget;
class PreviewWidget;
class ParseScheduler;
class ScrollSync;
class RecentFiles;
class TocPanel;
class QSplitter;
class QMenu;
class QActionGroup;
class QLocalServer;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);

    void openFile(const QString& path);
    void newTab();
    void restoreSession(const QString& requestedFile = QString());
    void startLocalServer(const char* serverName);

protected:
    bool eventFilter(QObject* obj, QEvent* event) override;
    void showEvent(QShowEvent* event) override;
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dropEvent(QDropEvent* event) override;
    void closeEvent(QCloseEvent* event) override;

private slots:
    void onNewFile();
    void onOpenFile();
    void onSaveFile();
    void onSaveFileAs();
    void onCloseTab(int index);
    void onTabChanged(int index);

private:
    struct TabData {
        QSplitter* splitter = nullptr;
        EditorWidget* editor = nullptr;
        PreviewWidget* preview = nullptr;
        ParseScheduler* scheduler = nullptr;
        ScrollSync* scrollSync = nullptr;
        bool pendingReload = false;  // 文件被外部修改，等切换到此 tab 时提示
    };

    QTabWidget* m_tabWidget;
    QSplitter* m_mainSplitter = nullptr;
    TocPanel* m_tocPanel = nullptr;
    QVector<TabData> m_tabs;
    RecentFiles* m_recentFiles;
    QMenu* m_recentMenu = nullptr;

    Theme m_currentTheme;
    QLocalServer* m_localServer = nullptr;

    // 持久化的视图设置
    QAction* m_restoreSessionAct = nullptr;
    QAction* m_wordWrapAct = nullptr;
    QAction* m_followSystemThemeAct = nullptr;
    QAction* m_lightThemeAct = nullptr;
    QAction* m_darkThemeAct = nullptr;
    QVector<QAction*> m_spacingActions;
    qreal m_lineSpacingFactor = 1.0;

    void saveSettings();
    void saveSessionLater();
    void loadSettings();
#ifdef _WIN32
    void setDarkTitleBar(bool dark);
#endif
    QTimer m_saveSessionTimer;
    QByteArray m_pendingSplitterState;
    bool m_splitterInitialized = false;

    QFileSystemWatcher m_fileWatcher;
    void watchFile(const QString& path);
    void unwatchFile(const QString& path);
    void onFileChangedExternally(const QString& path);
    void promptReloadTab(int tabIndex);

    void setupMenuBar();
    void setupDragDrop();

    // 导出与打印
    QString currentMarkdownToHtml();
    void onExportHtml();
    void onExportPdf();
    void onPrint();

    // 字体缩放
    int m_fontSizeDelta = 0;  // 相对于默认字号的偏移
    void zoomIn();
    void zoomOut();
    void zoomReset();
    void applyFontSize();
    TabData createTab();
    void updateTabTitle(int index);
    void setTabCloseButton(int index);
    void updateAllTabCloseButtons();
    void updateRecentFilesMenu();
    void applyTheme(const Theme& theme);
    void applySystemTheme();
    bool isSystemDarkMode() const;
    TabData* currentTab();
    bool maybeSave(int index);
};
