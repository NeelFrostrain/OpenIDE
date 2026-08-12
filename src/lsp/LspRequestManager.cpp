#include "lsp/LspRequestManager.h"
#include "core/Logger.h"

namespace OpenIDE::Lsp {

LspRequestManager::LspRequestManager(QObject* parent)
    : QObject(parent) {
}

int LspRequestManager::createRequest(const std::string& method,
                                      const std::filesystem::path& path,
                                      int version,
                                      uint64_t generation,
                                      std::function<void(const nlohmann::json&)> callback) {
    int id = m_nextId++;
    PendingRequest req;
    req.id = id;
    req.method = method;
    req.path = path;
    req.documentVersion = version;
    req.generation = generation;
    req.callback = callback;

    m_pendingRequests[id] = req;
    return id;
}

bool LspRequestManager::validateAndDispatch(int id, const nlohmann::json& responseResult) {
    auto it = m_pendingRequests.find(id);
    if (it == m_pendingRequests.end()) {
        OpenIDE::Core::Logger::instance().debug("LspRequestManager", QString("Discarded response for unknown or cancelled req #%1").arg(id));
        return false;
    }

    PendingRequest req = it->second;
    m_pendingRequests.erase(it);

    if (req.generation != m_generation.load()) {
        OpenIDE::Core::Logger::instance().info("LspRequestManager", QString("[LSP] Response discarded id=%1 reason=stale generation (%2 != %3)")
            .arg(id).arg(req.generation).arg(m_generation.load()));
        return false;
    }

    if (req.callback) {
        req.callback(responseResult);
        return true;
    }

    return false;
}

void LspRequestManager::cancelAll() {
    m_generation++;
    m_pendingRequests.clear();
    OpenIDE::Core::Logger::instance().info("LspRequestManager", QString("[LSP] Cancelled all pending requests (New generation: %1)").arg(m_generation.load()));
}

} // namespace OpenIDE::Lsp
