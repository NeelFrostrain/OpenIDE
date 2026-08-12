#include "app/Application.h"
#include "ui/MainWindow.h"
#include "core/Logger.h"

int main(int argc, char* argv[]) {
    OpenIDE::App::Application app(argc, argv);
    if (!app.init()) {
        return 1;
    }

    OpenIDE::UI::MainWindow mainWindow;
    mainWindow.show();

    return app.exec();
}
