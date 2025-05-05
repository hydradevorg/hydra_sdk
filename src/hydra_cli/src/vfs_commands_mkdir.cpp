#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <map>

#include "../include/commands/vfs_commands.h"
#include <hydra_vfs/vfs.h>
#include <hydra_vfs/container_vfs.h>

namespace hydra {
namespace cli {
namespace vfs {

int MkdirCommand::execute(const std::vector<std::string>& args) {
    if (is_help_requested(args)) {
        print_help();
        return 0;
    }
    
    std::cout << "Mkdir command not yet implemented" << std::endl;
    return 1;
}

void MkdirCommand::print_help() const {
    std::cout << "Usage: hydra vfs mkdir [OPTIONS] CONTAINER_PATH DIRECTORY_PATH" << std::endl;
    std::cout << "Create a new directory in the container" << std::endl;
    std::cout << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  --key-file PATH       Use key file instead of password" << std::endl;
    std::cout << "  --password PASS       Use password for container" << std::endl;
    std::cout << "  -p, --parents         Create parent directories as needed" << std::endl;
    std::cout << "  -h, --help            Show this help message" << std::endl;
}

} // namespace vfs
} // namespace cli
} // namespace hydra
