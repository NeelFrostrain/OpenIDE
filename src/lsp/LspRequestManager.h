#pragma once

#include "lsp/LspTypes.h"
#include <QObject>
#include <unordered_map>
#include <atomic>
#include <functional>
#include <nlohmann/json.hpp>

namespace OpenIDE::Lsp {

struct PendingRequest {
    int id = 0;
    std::string method;
    std::filesystem::path path;
    int documentVersion = 0;
    uint64_t generation = 0;
    std::function<void(const nlohmann::json&)> callback;
};

class LspRequestManager : public QObject {
    Q_OBJECT

public:
    explicit LspRequestManager(QObject* parent = nullptr);

    int createRequest(const std::string& method,
                      const std::filesystem::path& path,
                      int version,
                      uint64_t generation,
                      std::function<void(const nlohmann::json&)> callback);

    bool validateAndDispatch(int id, const nlohmann::json& responseResult);
    void cancelAll();
    uint64_t currentGeneration() const { return m_generation.load(); }
    uint64_t nextGeneration() { return ++m_generation; }

private:
    std::atomic<int> m_nextId{1};
    std::atomic<uint64_t> m_generation{1};
    std::unordered_map<int, PendingRequest> m_pendingRequests;
};

} // namespace OpenIDE::Lsp
