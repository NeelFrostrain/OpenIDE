#pragma once

#include <QApplication>
#include <memory>

namespace MyIDE::App {

class Application : public QApplication {
    Q_OBJECT

public:
    Application(int& argc, char** argv);
    ~Application();

    bool init();
};

} // namespace MyIDE::App
