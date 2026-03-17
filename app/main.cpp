#include <QApplication>
#include <QMainWindow>
#include "EditorWidget.h"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    QMainWindow window;
    window.setWindowTitle("EasyMarkdown");
    window.resize(1024, 768);

    auto* editor = new EditorWidget(&window);
    window.setCentralWidget(editor);

    window.show();
    return app.exec();
}
