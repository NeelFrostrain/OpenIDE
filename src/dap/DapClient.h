#pragma once

#include <QObject>
#include <QProcess>
#include <QList>
#include "DapTypes.h"

class DapClient : public QObject {
    Q_OBJECT
public:
    explicit DapClient(QObject* parent = nullptr);

    void addBreakpoint(const QString& filePath, int line);
    void removeBreakpoint(const QString& filePath, int line);
    QList<BreakpointItem> breakpointsForFile(const QString& filePath) const;
    bool hasBreakpoint(const QString& filePath, int line) const;

    void startDebugging(const QString& programPath);
    void stopDebugging();
    void pause();
    void stepOver();
    void stepInto();
    void stepOut();

signals:
    void targetStopped(const QString& file, int line);
    void targetResumed();
    void variablesUpdated(const QList<VariableItem>& vars);
    void callStackUpdated(const QList<StackFrameItem>& frames);

private:
    QList<BreakpointItem> m_breakpoints;
    bool m_isDebugging{false};
};
