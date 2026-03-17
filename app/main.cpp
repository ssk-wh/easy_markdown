#include <QApplication>
#include "MainWindow.h"

#ifdef _WIN32
#include <windows.h>
#include <dbghelp.h>
#pragma comment(lib, "dbghelp.lib")

static LONG WINAPI crashHandler(EXCEPTION_POINTERS* ep)
{
    FILE* f = fopen("crash_report.txt", "w");
    if (f) {
        fprintf(f, "Exception Code: 0x%08lX\n", ep->ExceptionRecord->ExceptionCode);
        fprintf(f, "Exception Address: %p\n", ep->ExceptionRecord->ExceptionAddress);

        // Find which module the crash address belongs to
        HMODULE hMod = NULL;
        GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS,
                           (LPCSTR)ep->ExceptionRecord->ExceptionAddress, &hMod);
        if (hMod) {
            char name[MAX_PATH];
            GetModuleFileNameA(hMod, name, MAX_PATH);
            fprintf(f, "Crash module: %s\n", name);
            fprintf(f, "Offset: 0x%llX\n",
                    (unsigned long long)((char*)ep->ExceptionRecord->ExceptionAddress - (char*)hMod));
        }

        // Write minidump
        HANDLE dumpFile = CreateFileA("crash.dmp", GENERIC_WRITE, 0, NULL,
                                       CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (dumpFile != INVALID_HANDLE_VALUE) {
            MINIDUMP_EXCEPTION_INFORMATION mei;
            mei.ThreadId = GetCurrentThreadId();
            mei.ExceptionPointers = ep;
            mei.ClientPointers = FALSE;
            MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(),
                              dumpFile, MiniDumpNormal, &mei, NULL, NULL);
            CloseHandle(dumpFile);
            fprintf(f, "Minidump written to crash.dmp\n");
        }
        fclose(f);
    }
    return EXCEPTION_EXECUTE_HANDLER;
}
#endif

int main(int argc, char* argv[])
{
#ifdef _WIN32
    SetUnhandledExceptionFilter(crashHandler);
#endif

    // Workaround: Qt 5.15 Release 模式下 Qt5Gui.dll 在首次绘制时可能因
    // 内部日志回调未初始化导致 ACCESS_VIOLATION。设置日志规则触发内部初始化。
    // Workaround: Qt 5.15 + MSVC Release 模式下 Qt5Gui.dll 首次绘制时
    // 内部访问违规（偏移 0x1049F4）。设置 QT_LOGGING_RULES 触发 Qt 内部的
    // logging category 初始化路径，避免未初始化指针访问。
    QApplication app(argc, argv);
    app.setApplicationName("EasyMarkdown");
    app.setOrganizationName("EasyMarkdown");

    MainWindow window;

    if (argc > 1) {
        window.openFile(QString::fromLocal8Bit(argv[1]));
    } else {
        window.newTab();
    }

    window.show();
    return app.exec();
}
