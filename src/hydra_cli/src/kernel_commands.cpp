#include "../include/commands/kernel_commands.h"
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <fstream>

// Use experimental/filesystem for older compilers or filesystem for C++17 and later
#if __cplusplus >= 201703L
#include <filesystem>
namespace fs = std::filesystem;
#else
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#endif

namespace hydra {
namespace cli {
namespace kernel {

int KernelCommand::execute(const std::vector<std::string>& args) {
    if (is_help_requested(args)) {
        print_help();
        return 0;
    }

    // Need at least one argument (the subcommand)
    if (args.empty()) {
        std::cerr << "Error: Missing subcommand" << std::endl;
        print_help();
        return 1;
    }

    std::string subcommand = args[0];
    std::vector<std::string> subcommand_args(args.begin() + 1, args.end());
    
    if (subcommand == "run") {
        return m_run_command.execute(subcommand_args);
    } else if (subcommand == "list") {
        return m_list_command.execute(subcommand_args);
    } else if (subcommand == "stop") {
        return m_stop_command.execute(subcommand_args);
    } else if (subcommand == "info") {
        return m_info_command.execute(subcommand_args);
    } else if (subcommand == "exec") {
        return m_exec_command.execute(subcommand_args);
    } else if (subcommand == "attach") {
        return m_attach_command.execute(subcommand_args);
    } else if (subcommand == "port") {
        return m_port_command.execute(subcommand_args);
    } else if (subcommand == "resource") {
        return m_resource_command.execute(subcommand_args);
    } else if (subcommand == "init") {
        return m_init_command.execute(subcommand_args);
    } else {
        std::cerr << "Error: Unknown subcommand '" << subcommand << "'" << std::endl;
        print_help();
        return 1;
    }
}

void KernelCommand::print_help() const {
    std::cout << "Hydra CLI - Kernel Commands" << std::endl;
    std::cout << "Usage: hydra-cli kernel <subcommand> [options]" << std::endl;
    std::cout << std::endl;
    std::cout << "Subcommands:" << std::endl;
    std::cout << "  run <config>      Run a containerized application with the kernel" << std::endl;
    std::cout << "  list              List all running kernels" << std::endl;
    std::cout << "  stop <id>         Stop a running kernel" << std::endl;
    std::cout << "  info <id>         Display information about a running kernel" << std::endl;
    std::cout << "  exec <id> <cmd>   Execute a command in a running kernel process" << std::endl;
    std::cout << "  attach <id>       Attach to a running kernel process" << std::endl;
    std::cout << "  port <id> <cmd>   Manage port forwarding" << std::endl;
    std::cout << "  resource <id> <cmd> Manage resource limits" << std::endl;
    std::cout << "  init <name>       Generate a sample configuration file" << std::endl;
    std::cout << std::endl;
    std::cout << "For detailed help on a specific subcommand, use:" << std::endl;
    std::cout << "  hydra-cli kernel <subcommand> --help" << std::endl;
}

// Default implementation for other commands
int ListCommand::execute(const std::vector<std::string>& args) {
    if (is_help_requested(args)) {
        print_help();
        return 0;
    }
    
    std::cout << "Not implemented yet" << std::endl;
    return 0;
}

void ListCommand::print_help() const {
    std::cout << "Hydra CLI - Kernel List Command" << std::endl;
    std::cout << "Usage: hydra-cli kernel list" << std::endl;
    std::cout << std::endl;
    std::cout << "Description:" << std::endl;
    std::cout << "  Lists all running kernels" << std::endl;
}

int StopCommand::execute(const std::vector<std::string>& args) {
    if (is_help_requested(args)) {
        print_help();
        return 0;
    }
    
    std::cout << "Not implemented yet" << std::endl;
    return 0;
}

void StopCommand::print_help() const {
    std::cout << "Hydra CLI - Kernel Stop Command" << std::endl;
    std::cout << "Usage: hydra-cli kernel stop <id>" << std::endl;
    std::cout << std::endl;
    std::cout << "Description:" << std::endl;
    std::cout << "  Stops a running kernel" << std::endl;
    std::cout << std::endl;
    std::cout << "Arguments:" << std::endl;
    std::cout << "  id    ID of the kernel to stop" << std::endl;
}

int InfoCommand::execute(const std::vector<std::string>& args) {
    if (is_help_requested(args)) {
        print_help();
        return 0;
    }
    
    std::cout << "Not implemented yet" << std::endl;
    return 0;
}

void InfoCommand::print_help() const {
    std::cout << "Hydra CLI - Kernel Info Command" << std::endl;
    std::cout << "Usage: hydra-cli kernel info <id>" << std::endl;
    std::cout << std::endl;
    std::cout << "Description:" << std::endl;
    std::cout << "  Displays information about a running kernel" << std::endl;
    std::cout << std::endl;
    std::cout << "Arguments:" << std::endl;
    std::cout << "  id    ID of the kernel to query" << std::endl;
}

int ExecCommand::execute(const std::vector<std::string>& args) {
    if (is_help_requested(args)) {
        print_help();
        return 0;
    }
    
    std::cout << "Not implemented yet" << std::endl;
    return 0;
}

void ExecCommand::print_help() const {
    std::cout << "Hydra CLI - Kernel Exec Command" << std::endl;
    std::cout << "Usage: hydra-cli kernel exec <id> <command> [args...]" << std::endl;
    std::cout << std::endl;
    std::cout << "Description:" << std::endl;
    std::cout << "  Executes a command in a running kernel process" << std::endl;
    std::cout << std::endl;
    std::cout << "Arguments:" << std::endl;
    std::cout << "  id        ID of the kernel" << std::endl;
    std::cout << "  command   Command to execute" << std::endl;
    std::cout << "  args      Command arguments" << std::endl;
}

int AttachCommand::execute(const std::vector<std::string>& args) {
    if (is_help_requested(args)) {
        print_help();
        return 0;
    }
    
    std::cout << "Not implemented yet" << std::endl;
    return 0;
}

void AttachCommand::print_help() const {
    std::cout << "Hydra CLI - Kernel Attach Command" << std::endl;
    std::cout << "Usage: hydra-cli kernel attach <id>" << std::endl;
    std::cout << std::endl;
    std::cout << "Description:" << std::endl;
    std::cout << "  Attaches to a running kernel process" << std::endl;
    std::cout << std::endl;
    std::cout << "Arguments:" << std::endl;
    std::cout << "  id    ID of the kernel to attach to" << std::endl;
}

int PortCommand::execute(const std::vector<std::string>& args) {
    if (is_help_requested(args)) {
        print_help();
        return 0;
    }
    
    std::cout << "Not implemented yet" << std::endl;
    return 0;
}

void PortCommand::print_help() const {
    std::cout << "Hydra CLI - Kernel Port Command" << std::endl;
    std::cout << "Usage: hydra-cli kernel port <id> <command> [args...]" << std::endl;
    std::cout << std::endl;
    std::cout << "Description:" << std::endl;
    std::cout << "  Manages port forwarding for a running kernel" << std::endl;
    std::cout << std::endl;
    std::cout << "Commands:" << std::endl;
    std::cout << "  list              List all port mappings" << std::endl;
    std::cout << "  add <int> <ext>   Add a new port mapping" << std::endl;
    std::cout << "  remove <int>      Remove a port mapping" << std::endl;
    std::cout << "  bind <int> <pid>  Bind a port to a process" << std::endl;
}

int ResourceCommand::execute(const std::vector<std::string>& args) {
    if (is_help_requested(args)) {
        print_help();
        return 0;
    }
    
    std::cout << "Not implemented yet" << std::endl;
    return 0;
}

void ResourceCommand::print_help() const {
    std::cout << "Hydra CLI - Kernel Resource Command" << std::endl;
    std::cout << "Usage: hydra-cli kernel resource <id> <command> [args...]" << std::endl;
    std::cout << std::endl;
    std::cout << "Description:" << std::endl;
    std::cout << "  Manages resource limits for a running kernel" << std::endl;
    std::cout << std::endl;
    std::cout << "Commands:" << std::endl;
    std::cout << "  list              List all resource limits" << std::endl;
    std::cout << "  set <pid> <res> <val> Set a resource limit" << std::endl;
    std::cout << "  get <pid> <res>   Get a resource limit" << std::endl;
}

int InitCommand::execute(const std::vector<std::string>& args) {
    if (is_help_requested(args)) {
        print_help();
        return 0;
    }
    
    // Need at least one argument (the configuration name)
    if (args.empty()) {
        std::cerr << "Error: Missing configuration name" << std::endl;
        print_help();
        return 1;
    }
    
    std::string config_name = args[0];
    std::string output_path = config_name + ".yml";
    
    // Check if output file already exists
    if (fs::exists(output_path)) {
        std::cout << "File already exists: " << output_path << std::endl;
        std::cout << "Do you want to overwrite it? (y/n): ";
        std::string response;
        std::getline(std::cin, response);
        if (response != "y" && response != "Y") {
            std::cout << "Operation cancelled" << std::endl;
            return 0;
        }
    }
    
    // Create sample configuration file
    std::ofstream output(output_path);
    if (!output) {
        std::cerr << "Error: Failed to create configuration file" << std::endl;
        return 1;
    }
    
    // Write sample configuration
    output << "version: '1.0'\n\n";
    output << "container:\n";
    output << "  name: " << config_name << "\n";
    output << "  path: ./" << config_name << ".hcon\n\n";
    
    output << "security:\n";
    output << "  encryption: kyber_aes\n";
    output << "  security_level: standard\n\n";
    
    output << "isolation:\n";
    output << "  mode: partial  # none, partial, complete\n";
    output << "  ports:\n";
    output << "    - internal: 8080\n";
    output << "      external: 80\n";
    output << "      protocol: tcp\n";
    output << "      process: web_server\n\n";
    
    output << "resources:\n";
    output << "  max_memory: 1GB\n";
    output << "  max_cpu: 0.5  # 50% CPU limit\n";
    output << "  max_files: 1000\n";
    output << "  max_file_size: 100MB\n\n";
    
    output << "processes:\n";
    output << "  - name: web_server\n";
    output << "    command: /bin/web_server\n";
    output << "    args:\n";
    output << "      - --port=8080\n";
    output << "    env:\n";
    output << "      PORT: 8080\n";
    output << "      DEBUG: \"false\"\n";
    output << "    limits:\n";
    output << "      memory: 512MB\n";
    output << "      cpu: 0.2\n\n";
    
    output << "mounts:\n";
    output << "  - source: ./data\n";
    output << "    target: /data\n";
    output << "    read_only: false\n";
    
    output.close();
    
    std::cout << "Created sample configuration: " << output_path << std::endl;
    std::cout << "You can now edit this file and then run:" << std::endl;
    std::cout << "  hydra-cli kernel run " << output_path << std::endl;
    
    return 0;
}

void InitCommand::print_help() const {
    std::cout << "Hydra CLI - Kernel Init Command" << std::endl;
    std::cout << "Usage: hydra-cli kernel init <name>" << std::endl;
    std::cout << std::endl;
    std::cout << "Description:" << std::endl;
    std::cout << "  Generates a sample configuration file for kernel containerization" << std::endl;
    std::cout << std::endl;
    std::cout << "Arguments:" << std::endl;
    std::cout << "  name    Name for the configuration (will be used for the filename)" << std::endl;
}

} // namespace kernel
} // namespace cli
} // namespace hydra
