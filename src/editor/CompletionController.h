#pragma once

#include "editor/EditorWidget.h"
#include "editor/CompletionPopup.h"
#include "language/CompletionService.h"
#include "language/CompletionContext.h"
#include <QObject>
#include <filesystem>

namespace OpenIDE::Editor {

enum class CancelReason {
    Escape,
    CursorMoved,
    DocumentChanged,
    InvalidContext,
    FocusOut,
    TabChanged,
    DocumentClosed,
    NoResults,
    Accepted,
    StaleResponse,
    ApplicationInactive,
    WindowMinimized
};

struct CompletionSession {
    bool active = false;
    uint64_t generation = 0;
    std::filesystem::path path;
    int line = 1;
    int column = 0;
    QString prefix;
    Language::ContextKind contextKind = Language::ContextKind::GeneralCode;
};

class CompletionController : public QObject {
    Q_OBJECT

public:
    explicit CompletionController(Language::CompletionService* service, QObject* parent = nullptr);

    void attachEditor(EditorWidget* editor);
    void detachEditor();

    bool handleKeyPress(QKeyEvent* event);
    bool eventFilter(QObject* watched, QEvent* event) override;
    void triggerCompletion(bool isManual);
    void cancelSession(CancelReason reason);
    bool isSessionActive() const { return m_session.active; }

    uint64_t currentGeneration() const { return m_session.generation; }

private slots:
    void onCursorPositionChanged();
    void onTextChanged();
    void onCompletionItemSelected(const CompletionItemData& item);

private:
    void repositionPopup();

    Language::CompletionService* m_service = nullptr;
    EditorWidget* m_activeEditor = nullptr;
    CompletionPopup* m_popup = nullptr;

    CompletionSession m_session;
    int m_lastLine = -1;
    int m_lastCol = -1;
};

} // namespace OpenIDE::Editor
