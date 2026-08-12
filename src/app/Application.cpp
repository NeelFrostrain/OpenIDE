#include "app/Application.h"
#include "core/Logger.h"
#include "core/Config.h"

namespace OpenIDE::App {

Application::Application(int& argc, char** argv)
    : QApplication(argc, argv) {
    setApplicationName("OpenIDE");
    setOrganizationName("OpenIDE");
    setApplicationVersion("0.1.0");
}

Application::~Application() {
    OpenIDE::Core::Logger::instance().info("App", "Application shutting down");
}

bool Application::init() {
    OpenIDE::Core::Logger::instance().init();
    OpenIDE::Core::Logger::instance().info("App", "Initializing OpenIDE Application...");
    OpenIDE::Core::Config::instance().load({});
    return true;
}

} // namespace OpenIDE::App
