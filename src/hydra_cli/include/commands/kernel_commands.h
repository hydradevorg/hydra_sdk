#ifndef HYDRA_CLI_KERNEL_COMMANDS_H
#define HYDRA_CLI_KERNEL_COMMANDS_H

#include "command.h"
#include <string>
#include <vector>

namespace hydra {
namespace cli {
namespace kernel {

/**
 * @brief Command to run a kernel with Docker-like configuration
 */
class RunCommand : public Command {
public:
    int execute(const std::vector<std::string>& args) override;
    void print_help() const override;
};

/**
 * @brief Command to list running kernels
 */
class ListCommand : public Command {
public:
    int execute(const std::vector<std::string>& args) override;
    void print_help() const override;
};

/**
 * @brief Command to stop a running kernel
 */
class StopCommand : public Command {
public:
    int execute(const std::vector<std::string>& args) override;
    void print_help() const override;
};

/**
 * @brief Command to get information about a running kernel
 */
class InfoCommand : public Command {
public:
    int execute(const std::vector<std::string>& args) override;
    void print_help() const override;
};

/**
 * @brief Command to execute a command in a running kernel process
 */
class ExecCommand : public Command {
public:
    int execute(const std::vector<std::string>& args) override;
    void print_help() const override;
};

/**
 * @brief Command to attach to a running kernel process
 */
class AttachCommand : public Command {
public:
    int execute(const std::vector<std::string>& args) override;
    void print_help() const override;
};

/**
 * @brief Command to manage port forwarding
 */
class PortCommand : public Command {
public:
    int execute(const std::vector<std::string>& args) override;
    void print_help() const override;
};

/**
 * @brief Command to manage resource limits
 */
class ResourceCommand : public Command {
public:
    int execute(const std::vector<std::string>& args) override;
    void print_help() const override;
};

/**
 * @brief Command to generate a sample configuration file
 */
class InitCommand : public Command {
public:
    int execute(const std::vector<std::string>& args) override;
    void print_help() const override;
};

/**
 * @brief Base command for kernel operations
 */
class KernelCommand : public Command {
public:
    int execute(const std::vector<std::string>& args) override;
    void print_help() const override;

private:
    RunCommand m_run_command;
    ListCommand m_list_command;
    StopCommand m_stop_command;
    InfoCommand m_info_command;
    ExecCommand m_exec_command;
    AttachCommand m_attach_command;
    PortCommand m_port_command;
    ResourceCommand m_resource_command;
    InitCommand m_init_command;
};

} // namespace kernel
} // namespace cli
} // namespace hydra

#endif // HYDRA_CLI_KERNEL_COMMANDS_H
