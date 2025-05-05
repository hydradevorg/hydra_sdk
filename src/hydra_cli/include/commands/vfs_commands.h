#pragma once

#include "command.h"
#include <hydra_vfs/vfs.h>
#include <hydra_vfs/container_vfs.h>
#include <memory>
#include <string>

namespace hydra {
namespace cli {
namespace vfs {

// Base class for VFS commands that need a container
class ContainerCommand : public Command {
public:
    int execute(const std::vector<std::string>& args) override;
    void print_help() const override;

protected:
    // Helper method to load an encryption key from file or password
    hydra::vfs::EncryptionKey load_encryption_key(const std::string& key_source, bool is_file) const;
};

// Command to list files and directories in a container
class ListCommand : public Command {
public:
    int execute(const std::vector<std::string>& args) override;
    void print_help() const override;

private:
    // Helper to open a container with the given path and key
    std::shared_ptr<hydra::vfs::IVirtualFileSystem> open_container(
        const std::string& container_path,
        const std::string& key_source,
        bool key_is_file
    ) const;
};

// Command to display file contents
class CatCommand : public Command {
public:
    int execute(const std::vector<std::string>& args) override;
    void print_help() const override;

private:
    std::shared_ptr<hydra::vfs::IVirtualFileSystem> open_container(
        const std::string& container_path,
        const std::string& key_source,
        bool key_is_file
    ) const;
};

// Command to put a file into a container
class PutCommand : public Command {
public:
    int execute(const std::vector<std::string>& args) override;
    void print_help() const override;

private:
    std::shared_ptr<hydra::vfs::IVirtualFileSystem> open_container(
        const std::string& container_path,
        const std::string& key_source,
        bool key_is_file
    ) const;
};

// Command to get a file from a container
class GetCommand : public Command {
public:
    int execute(const std::vector<std::string>& args) override;
    void print_help() const override;

private:
    std::shared_ptr<hydra::vfs::IVirtualFileSystem> open_container(
        const std::string& container_path,
        const std::string& key_source,
        bool key_is_file
    ) const;
};

// Command to remove a file or directory from a container
class RemoveCommand : public Command {
public:
    int execute(const std::vector<std::string>& args) override;
    void print_help() const override;

private:
    std::shared_ptr<hydra::vfs::IVirtualFileSystem> open_container(
        const std::string& container_path,
        const std::string& key_source,
        bool key_is_file
    ) const;
};

// Command to create a directory in a container
class MkdirCommand : public Command {
public:
    int execute(const std::vector<std::string>& args) override;
    void print_help() const override;

private:
    std::shared_ptr<hydra::vfs::IVirtualFileSystem> open_container(
        const std::string& container_path,
        const std::string& key_source,
        bool key_is_file
    ) const;
};

// Command to show statistics about a container
class StatsCommand : public Command {
public:
    int execute(const std::vector<std::string>& args) override;
    void print_help() const override;

private:
    std::shared_ptr<hydra::vfs::IVirtualFileSystem> open_container(
        const std::string& container_path,
        const std::string& key_source,
        bool key_is_file
    ) const;
};

} // namespace vfs
} // namespace cli
} // namespace hydra
