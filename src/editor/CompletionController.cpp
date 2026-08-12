#include "editor/CompletionController.h"
#include "editor/snippets/SnippetEngine.h"
#include "core/Logger.h"
#include <QApplication>

namespace OpenIDE::Editor {

static const char* cancelReasonToString(CancelReason reason) {
    switch (reason) {
        case CancelReason::Escape: return "Escape";
        case CancelReason::CursorMoved: return "CursorMoved";
        case CancelReason::DocumentChanged: return "DocumentChanged";
        case CancelReason::InvalidContext: return "InvalidContext";
        case CancelReason::FocusOut: return "FocusOut";
        case CancelReason::TabChanged: return "TabChanged";
        case CancelReason::DocumentClosed: return "DocumentClosed";
        case CancelReason::NoResults: return "NoResults";
        case CancelReason::Accepted: return "Accepted";
        case CancelReason::StaleResponse: return "StaleResponse";
    }
    return "Unknown";
}

CompletionController::CompletionController(Language::CompletionService* service, QObject* parent)
    : QObject(parent), m_service(service) {
    m_popup = new CompletionPopup(nullptr);
    connect(m_popup, &CompletionPopup::itemSelected, this, &CompletionController::onCompletionItemSelected);

    connect(qApp, &QGuiApplication::applicationStateChanged, [this](Qt::ApplicationState state) {
        if (state != Qt::ApplicationActive) {
            cancelSession(CancelReason::ApplicationInactive);
        }
    });
}

bool CompletionController::handleKeyPress(QKeyEvent* event) {
    if (!m_session.active || !m_popup || !m_popup->isVisible()) {
        if (m_activeEditor && Snippets::SnippetEngine::instance().isSessionActive()) {
            return Snippets::SnippetEngine::instance().handleKeyEvent(m_activeEditor, event);
        }
        return false;
    }

    int key = event->key();

    if (key == Qt::Key_Down) {
        m_popup->moveSelectionDown();
        OpenIDE::Core::Logger::instance().info("Completion", QString("[Session] Navigate Down item='%1'").arg(m_popup->currentItemData().label));
        return true;
    } else if (key == Qt::Key_Up) {
        m_popup->moveSelectionUp();
        OpenIDE::Core::Logger::instance().info("Completion", QString("[Session] Navigate Up item='%1'").arg(m_popup->currentItemData().label));
        return true;
    } else if (key == Qt::Key_PageDown) {
        m_popup->moveSelectionPageDown();
        return true;
    } else if (key == Qt::Key_PageUp) {
        m_popup->moveSelectionPageUp();
        return true;
    } else if (key == Qt::Key_Home) {
        m_popup->selectHome();
        return true;
    } else if (key == Qt::Key_End) {
        m_popup->selectEnd();
        return true;
    } else if (key == Qt::Key_Return || key == Qt::Key_Enter || key == Qt::Key_Tab) {
        auto item = m_popup->currentItemData();
        if (!item.label.isEmpty()) {
            OpenIDE::Core::Logger::instance().info("Completion", QString("[Session] Accept item='%1'").arg(item.label));
            onCompletionItemSelected(item);
        } else {
            cancelSession(CancelReason::Accepted);
        }
        return true;
    } else if (key == Qt::Key_Escape) {
        cancelSession(CancelReason::Escape);
        return true;
    }

    return false;
}

void CompletionController::attachEditor(EditorWidget* editor) {
    if (m_activeEditor == editor) return;

    if (m_activeEditor) {
        detachEditor();
    }

    m_activeEditor = editor;
    if (!m_activeEditor) return;

    m_activeEditor->setCompletionController(this);
    m_activeEditor->installEventFilter(this);

    connect(m_activeEditor, &QPlainTextEdit::cursorPositionChanged, this, &CompletionController::onCursorPositionChanged);
    connect(m_activeEditor, &QPlainTextEdit::textChanged, this, &CompletionController::onTextChanged);

    cancelSession(CancelReason::TabChanged);
}

void CompletionController::detachEditor() {
    if (m_activeEditor) {
        m_activeEditor->removeEventFilter(this);
        m_activeEditor->setCompletionController(nullptr);
        disconnect(m_activeEditor, nullptr, this, nullptr);
        m_activeEditor = nullptr;
    }
    cancelSession(CancelReason::TabChanged);
}

bool CompletionController::eventFilter(QObject* watched, QEvent* event) {
    if (watched == m_activeEditor && event->type() == QEvent::KeyPress && m_session.active && m_popup && m_popup->isVisible()) {
        QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
        if (handleKeyPress(keyEvent)) {
            return true; // Stop event propagation so QPlainTextEdit NEVER receives Enter/Tab/Navigation keys
        }
    }
    return QObject::eventFilter(watched, event);
}

void CompletionController::cancelSession(CancelReason reason) {
    if (m_session.active || m_popup->isVisible()) {
        m_session.active = false;
        m_session.generation++;
        m_popup->hide();
        OpenIDE::Core::Logger::instance().info("CompletionController", QString("[Session] CANCELLED generation=%1 reason=%2")
            .arg(m_session.generation)
            .arg(cancelReasonToString(reason)));
    }
}

void CompletionController::triggerCompletion(bool isManual) {
    if (!m_activeEditor || !m_service) return;

    QTextCursor cursor = m_activeEditor->textCursor();
    int line = cursor.blockNumber() + 1;
    int col = cursor.positionInBlock();
    QString linePrefix = cursor.block().text().left(col);

    auto ctx = Language::ContextAnalyzer::analyze(linePrefix, isManual);

    // Suppress automatic completion if prefix is empty and not member access or include path
    if (!isManual && ctx.typedPrefix.isEmpty() && ctx.kind == Language::ContextKind::GeneralCode) {
        cancelSession(CancelReason::InvalidContext);
        return;
    }

    // Suppress completion in invalid contexts unless manual trigger
    if ((ctx.kind == Language::ContextKind::Comment || ctx.kind == Language::ContextKind::StringLiteral) && !isManual) {
        cancelSession(CancelReason::InvalidContext);
        return;
    }

    m_session.active = true;
    m_session.generation++;
    uint64_t reqGen = m_session.generation;
    m_session.path = m_activeEditor->filePath();
    m_session.line = line;
    m_session.column = col;
    m_session.prefix = ctx.typedPrefix;
    m_session.contextKind = ctx.kind;

    m_lastLine = line;
    m_lastCol = col;

    OpenIDE::Core::Logger::instance().info("CompletionController", QString("[Session] START generation=%1 file=%2 pos=%3:%4 prefix='%5'")
        .arg(reqGen)
        .arg(QString::fromStdString(m_session.path.filename().string()))
        .arg(line).arg(col).arg(m_session.prefix));

    Language::CompletionRequestParams params;
    params.path = m_session.path;
    params.line = line;
    params.column = col;
    params.linePrefix = linePrefix;
    params.documentText = m_activeEditor->text();
    params.triggerKind = isManual ? Language::CompletionTriggerKind::Invoked : Language::CompletionTriggerKind::TriggerCharacter;

    m_service->requestCompletion(params, [this, reqGen](const std::vector<CompletionItemData>& items, int requestId) {
        Q_UNUSED(requestId);
        // Stale response race condition check
        if (!m_session.active || reqGen != m_session.generation) {
            OpenIDE::Core::Logger::instance().info("CompletionController", QString("[Session] DISCARDED response for generation=%1 (current=%2, active=%3)")
                .arg(reqGen).arg(m_session.generation).arg(m_session.active));
            return;
        }

        if (items.empty()) {
            cancelSession(CancelReason::NoResults);
            return;
        }

        m_popup->setCompletions(items);
        if (m_popup->hasItems()) {
            repositionPopup();
            m_popup->show();
            OpenIDE::Core::Logger::instance().info("CompletionController", QString("[Session] SHOWING popup for generation=%1 with %2 items")
                .arg(reqGen).arg(items.size()));
        } else {
            cancelSession(CancelReason::NoResults);
        }
    });
}

void CompletionController::repositionPopup() {
    if (!m_activeEditor) return;
    QRect cRect = m_activeEditor->cursorRect();
    QPoint globalPos = m_activeEditor->viewport()->mapToGlobal(cRect.bottomLeft());
    m_popup->move(globalPos);
}

void CompletionController::onCursorPositionChanged() {
    if (!m_activeEditor || !m_session.active) return;

    QTextCursor cursor = m_activeEditor->textCursor();
    int line = cursor.blockNumber() + 1;
    int col = cursor.positionInBlock();

    // If cursor jumped to another line or moved significantly backwards
    if (m_lastLine != -1 && (line != m_lastLine || abs(col - m_lastCol) > 5)) {
        cancelSession(CancelReason::CursorMoved);
    }
}

void CompletionController::onTextChanged() {
    if (!m_activeEditor || !m_session.active) return;

    QTextCursor cursor = m_activeEditor->textCursor();
    int line = cursor.blockNumber() + 1;
    int col = cursor.positionInBlock();
    QString linePrefix = cursor.block().text().left(col);

    auto ctx = Language::ContextAnalyzer::analyze(linePrefix, false);
    if (ctx.kind == Language::ContextKind::Comment || ctx.kind == Language::ContextKind::StringLiteral) {
        cancelSession(CancelReason::InvalidContext);
        return;
    }

    // Refresh active session for new prefix
    triggerCompletion(false);
}

void CompletionController::onCompletionItemSelected(const CompletionItemData& item) {
    if (!m_activeEditor) return;

    int prefixLen = m_session.prefix.length();

    if (item.isSnippet) {
        cancelSession(CancelReason::Accepted);
        Snippets::SnippetEngine::instance().expandSnippetTemplate(m_activeEditor, item.insertText, prefixLen);
        return;
    }

    QTextCursor cursor = m_activeEditor->textCursor();
    int col = cursor.positionInBlock();
    QString lineText = cursor.block().text().left(col);

    if (prefixLen > 0 && lineText.endsWith(m_session.prefix)) {
        for (int i = 0; i < prefixLen; ++i) {
            cursor.deletePreviousChar();
        }
    }

    QString replacement = item.insertText.isEmpty() ? item.label : item.insertText;

    if ((item.kind == 2 || item.kind == 3) && !replacement.contains("(")) {
        replacement += "()";
    }

    cursor.insertText(replacement);
    m_activeEditor->setTextCursor(cursor);
    cancelSession(CancelReason::Accepted);
}

} // namespace OpenIDE::Editor
