#pragma once

#include <QDialog>
#include <QLineEdit>
#include <QListWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <filesystem>
#include <vector>

namespace OpenIDE::UI {

struct SearchResultItem {
    QString title;
    QString subtitle;
    QString category; // "File", "Class", "Symbol", "Action"
    std::filesystem::path path;
    int line = 0;
};

class SearchEverywhereDialog : public QDialog {
    Q_OBJECT

public:
    explicit SearchEverywhereDialog(QWidget* parent = nullptr);

    void setFiles(const std::vector<std::filesystem::path>& files);

signals:
    void fileSelected(const std::filesystem::path& path, int line);
    void actionTriggered(const QString& actionName);

protected:
    void keyPressEvent(QKeyEvent* event) override;

private slots:
    void onSearchTextChanged(const QString& text);
    void onItemActivated(QListWidgetItem* item);

private:
    QLineEdit* m_searchInput = nullptr;
    QListWidget* m_resultsList = nullptr;
    std::vector<std::filesystem::path> m_files;
    std::vector<SearchResultItem> m_currentResults;
};

} // namespace OpenIDE::UI
