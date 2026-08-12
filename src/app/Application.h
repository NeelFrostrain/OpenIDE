#pragma once

#include <QApplication>
#include <memory>

namespace OpenIDE::App {

class Application : public QApplication {
    Q_OBJECT

public:
    Application(int& argc, char** argv);
    ~Application();

    bool init();
};

} // namespace OpenIDE::App
