#pragma once

#include <QWidget>
#include <QListWidget>
#include <QTextEdit>
#include <QPushButton>
#include "vcs/GitManager.h"

class GitPanel : public QWidget {
    Q_OBJECT
public:
    explicit GitPanel(GitManager* gitMgr, QWidget* parent = nullptr);

    void setRepositoryPath(const QString& repoPath);
    void refreshStatus();

signals:
    void fileSelected(const QString& filePath);

private slots:
    void onCommitClicked();
    void onItemClicked(QListWidgetItem* item);

private:
    GitManager* m_gitMgr;
    QString m_repoPath;
    QListWidget* m_fileList;
    QTextEdit* m_diffViewer;
    QTextEdit* m_commitMsgEdit;
    QPushButton* m_commitButton;
};
