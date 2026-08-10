#include "DapClient.h"
#include <algorithm>

DapClient::DapClient(QObject* parent) : QObject(parent) {}

void DapClient::addBreakpoint(const QString& filePath, int line) {
    if (!hasBreakpoint(filePath, line)) {
        m_breakpoints.append({filePath, line, true});
    }
}

void DapClient::removeBreakpoint(const QString& filePath, int line) {
    auto it = std::remove_if(m_breakpoints.begin(), m_breakpoints.end(), [&](const BreakpointItem& b) {
        return b.filePath == filePath && b.line == line;
    });
    m_breakpoints.erase(it, m_breakpoints.end());
}

QList<BreakpointItem> DapClient::breakpointsForFile(const QString& filePath) const {
    QList<BreakpointItem> res;
    for (const auto& b : m_breakpoints) {
        if (b.filePath == filePath) res.append(b);
    }
    return res;
}

bool DapClient::hasBreakpoint(const QString& filePath, int line) const {
    for (const auto& b : m_breakpoints) {
        if (b.filePath == filePath && b.line == line) return true;
    }
    return false;
}

void DapClient::startDebugging(const QString& programPath) {
    Q_UNUSED(programPath);
    m_isDebugging = true;
    emit targetResumed();
}

void DapClient::stopDebugging() {
    m_isDebugging = false;
    emit targetResumed();
}

void DapClient::pause() {}
void DapClient::stepOver() {}
void DapClient::stepInto() {}
void DapClient::stepOut() {}
