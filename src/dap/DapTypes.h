#pragma once

#include <QString>
#include <QList>

struct BreakpointItem {
    QString filePath;
    int line{0};
    bool enabled{true};
};

struct StackFrameItem {
    int id{0};
    QString name;
    QString filePath;
    int line{0};
};

struct VariableItem {
    QString name;
    QString value;
    QString type;
};
