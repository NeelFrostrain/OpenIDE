#include "GitPanel.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QFileInfo>
#include <QMessageBox>

GitPanel::GitPanel(GitManager* gitMgr, QWidget* parent)
    : QWidget(parent), m_gitMgr(gitMgr) {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(4, 4, 4, 4);

    auto* splitter = new QSplitter(Qt::Vertical, this);

    // Changed files list
    m_fileList = new QListWidget(splitter);
    connect(m_fileList, &QListWidget::itemClicked, this, &GitPanel::onItemClicked);

    // Diff viewer
    m_diffViewer = new QTextEdit(splitter);
    m_diffViewer->setReadOnly(true);
    m_diffViewer->setFontFamily("Courier New");

    splitter->setSizes({120, 160});
    mainLayout->addWidget(splitter);

    // Commit section
    m_commitMsgEdit = new QTextEdit(this);
    m_commitMsgEdit->setPlaceholderText(tr("Commit message..."));
    m_commitMsgEdit->setMaximumHeight(60);
    mainLayout->addWidget(m_commitMsgEdit);

    m_commitButton = new QPushButton(tr("Commit Changes"), this);
    m_commitButton->setStyleSheet("background-color: #2e436e; color: #ffffff; padding: 6px; border-radius: 4px; font-weight: bold;");
    connect(m_commitButton, &QPushButton::clicked, this, &GitPanel::onCommitClicked);
    mainLayout->addWidget(m_commitButton);
}

void GitPanel::setRepositoryPath(const QString& repoPath) {
    m_repoPath = repoPath;
    refreshStatus();
}

void GitPanel::refreshStatus() {
    m_fileList->clear();
    m_diffViewer->clear();

    if (!m_gitMgr || m_repoPath.isEmpty()) return;

    QList<GitFileStatus> statuses = m_gitMgr->getStatus(m_repoPath);
    for (const auto& s : statuses) {
        auto* item = new QListWidgetItem(m_fileList);
        item->setText(QString("[%1] %2").arg(s.status, QFileInfo(s.filePath).fileName()));
        item->setData(Qt::UserRole, s.filePath);
    }
}

void GitPanel::onItemClicked(QListWidgetItem* item) {
    if (!item || !m_gitMgr) return;
    QString filePath = item->data(Qt::UserRole).toString();

    QString diff = m_gitMgr->getDiff(m_repoPath, filePath);
    m_diffViewer->setPlainText(diff);

    emit fileSelected(filePath);
}

void GitPanel::onCommitClicked() {
    if (!m_gitMgr || m_repoPath.isEmpty()) return;

    QString msg = m_commitMsgEdit->toPlainText().trimmed();
    if (msg.isEmpty()) {
        QMessageBox::warning(this, tr("Git Commit"), tr("Please enter a commit message."));
        return;
    }

    if (m_gitMgr->commit(m_repoPath, {}, msg)) {
        m_commitMsgEdit->clear();
        refreshStatus();
        QMessageBox::information(this, tr("Git Commit"), tr("Changes committed successfully!"));
    }
}
