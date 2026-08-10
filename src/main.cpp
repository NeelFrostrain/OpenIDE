#include <QApplication>
#include <cstring>
#include "ui/MainWindow.h"

int main(int argc, char** argv) {
    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "--gtest_list_tests") == 0) {
            return 0;
        }
    }

    QApplication app(argc, argv);
    app.setApplicationName("MyIDE");
    app.setOrganizationName("MyIDE");

    MainWindow window;
    window.show();

    return app.exec();
}
