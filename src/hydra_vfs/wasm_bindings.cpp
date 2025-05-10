#include <emscripten/bind.h>
#include <emscripten/val.h>

using namespace emscripten;

// Simple VFS implementation for testing
class P2PVFS {
public:
    P2PVFS(const std::string& nodeId, const std::string& storagePath)
        : nodeId(nodeId), storagePath(storagePath) {}

    bool addPeer(const std::string& peerId, const std::string& address) {
        peers[peerId] = address;
        return true;
    }

    std::vector<std::string> getPeers() {
        std::vector<std::string> result;
        for (const auto& [id, _] : peers) {
            result.push_back(id);
        }
        return result;
    }

    val createDirectory(const std::string& path) {
        val result = val::object();
        result.set("success", true);
        result.set("value", true);
        return result;
    }

    val listFiles(const std::string& path) {
        val result = val::object();
        result.set("success", true);

        std::vector<std::string> files = {"mock_file1.js", "mock_file2.js"};
        result.set("value", val::array(files));

        return result;
    }

    void synchronize() {
        // Mock implementation
    }

private:
    std::string nodeId;
    std::string storagePath;
    std::map<std::string, std::string> peers;
};

EMSCRIPTEN_BINDINGS(hydra_vfs) {
    class_<P2PVFS>("P2PVFS")
        .constructor<std::string, std::string>()
        .function("addPeer", &P2PVFS::addPeer)
        .function("getPeers", &P2PVFS::getPeers)
        .function("createDirectory", &P2PVFS::createDirectory)
        .function("listFiles", &P2PVFS::listFiles)
        .function("synchronize", &P2PVFS::synchronize);
}
