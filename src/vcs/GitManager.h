#pragma once

#include <QObject>
#include <QString>
#include <QStringList>
#include <QMap>

struct GitFileStatus {
    QString filePath;
    QString status; // "M" (Modified), "A" (Added), "?" (Untracked), "D" (Deleted)
};

class GitManager : public QObject {
    Q_OBJECT
public:
    explicit GitManager(QObject* parent = nullptr);

    bool isGitRepository(const QString& repoPath) const;
    QList<GitFileStatus> getStatus(const QString& repoPath);
    QString getDiff(const QString& repoPath, const QString& filePath = "");
    bool commit(const QString& repoPath, const QStringList& files, const QString& message);

signals:
    void gitStatusChanged();
};
