#include "app/Application.h"
#include "ui/MainWindow.h"
#include "core/Logger.h"

int main(int argc, char* argv[]) {
    MyIDE::App::Application app(argc, argv);
    if (!app.init()) {
        return 1;
    }

    MyIDE::UI::MainWindow mainWindow;
    mainWindow.show();

    return app.exec();
}
