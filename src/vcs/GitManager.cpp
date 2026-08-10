#include "GitManager.h"
#include <QProcess>
#include <QDir>
#include <QDebug>

GitManager::GitManager(QObject* parent) : QObject(parent) {}

bool GitManager::isGitRepository(const QString& repoPath) const {
    return QDir(repoPath + "/.git").exists();
}

QList<GitFileStatus> GitManager::getStatus(const QString& repoPath) {
    QList<GitFileStatus> results;
    if (!isGitRepository(repoPath)) return results;

    QProcess proc;
    proc.setWorkingDirectory(repoPath);
    proc.start("git", {"status", "--porcelain"});
    if (!proc.waitForFinished(3000)) return results;

    QString output = QString::fromUtf8(proc.readAllStandardOutput());
    const auto lines = output.split('\n', Qt::SkipEmptyParts);

    for (const auto& line : lines) {
        if (line.length() >= 4) {
            GitFileStatus s;
            s.status = line.left(2).trimmed();
            s.filePath = repoPath + "/" + line.mid(3).trimmed();
            results.append(s);
        }
    }

    return results;
}

QString GitManager::getDiff(const QString& repoPath, const QString& filePath) {
    if (!isGitRepository(repoPath)) return "";

    QProcess proc;
    proc.setWorkingDirectory(repoPath);
    QStringList args = {"diff"};
    if (!filePath.isEmpty()) {
        args.append(filePath);
    }
    proc.start("git", args);
    if (!proc.waitForFinished(3000)) return "";

    return QString::fromUtf8(proc.readAllStandardOutput());
}

bool GitManager::commit(const QString& repoPath, const QStringList& files, const QString& message) {
    if (!isGitRepository(repoPath) || message.trimmed().isEmpty()) return false;

    QProcess addProc;
    addProc.setWorkingDirectory(repoPath);
    QStringList addArgs = {"add"};
    if (files.isEmpty()) {
        addArgs.append(".");
    } else {
        addArgs.append(files);
    }
    addProc.start("git", addArgs);
    if (!addProc.waitForFinished(3000)) return false;

    QProcess commitProc;
    commitProc.setWorkingDirectory(repoPath);
    commitProc.start("git", {"commit", "-m", message});
    bool success = commitProc.waitForFinished(5000);

    if (success) {
        emit gitStatusChanged();
    }
    return success;
}
