#include "app/Application.h"
#include "core/Logger.h"
#include "core/Config.h"


namespace MyIDE::App {

Application::Application(int& argc, char** argv)
    : QApplication(argc, argv) {
    setApplicationName("MyIDE");
    setOrganizationName("MyIDE");
    setApplicationVersion("0.1.0");
}

Application::~Application() {
    MyIDE::Core::Logger::instance().info("App", "Application shutting down");
}

bool Application::init() {
    MyIDE::Core::Logger::instance().init();
    MyIDE::Core::Logger::instance().info("App", "Initializing MyIDE Application...");
    MyIDE::Core::Config::instance().load({});
    return true;
}

} // namespace MyIDE::App
