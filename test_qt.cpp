#include <QApplication>
#include <QMainWindow>
#include <QLabel>
#include <iostream>

int main(int argc, char* argv[])
{
    std::cout << "[1] Before QApplication" << std::endl;
    QApplication app(argc, argv);
    std::cout << "[2] After QApplication" << std::endl;

    QMainWindow window;
    std::cout << "[3] After QMainWindow" << std::endl;

    QLabel* label = new QLabel("Test");
    window.setCentralWidget(label);
    std::cout << "[4] After setCentralWidget" << std::endl;

    window.setWindowTitle("Qt Test");
    window.show();
    std::cout << "[5] After show" << std::endl;

    int result = app.exec();
    std::cout << "[6] After exec, result=" << result << std::endl;
    return result;
}
