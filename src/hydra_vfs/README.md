# Hydra Virtual File System (VFS)

A C++ library that creates an isolated virtual file system, similar to Docker containers but with a focus on security and isolation from the internet. This makes it suitable for sensitive operations like key management.

## Features

- **In-memory VFS**: Fully isolated virtual file system that exists only in memory
- **Persistent VFS**: Virtual file system that persists data to disk in an isolated location
- **Encrypted VFS**: Virtual file system with transparent encryption for all files
- **Container VFS**: Docker-like isolated container with hardware-level security features
- **Mount System**: Ability to mount one VFS inside another, creating a unified view
- **Isolation**: Complete isolation from the internet and external systems
- **C++ Standard Library**: Built using C++20 with standard library components

## Use Cases

- **Secure Key Management**: Store cryptographic keys in an isolated environment
- **Sandboxed File Operations**: Perform file operations in a controlled environment
- **Containerized Storage**: Create isolated storage areas for different applications
- **Testing**: Test file system operations without affecting the real file system
- **Secure Data Storage**: Store sensitive data with hardware-backed encryption

## Getting Started

### Prerequisites

- C++20 compatible compiler
- CMake 3.10 or higher
- OpenSSL for encryption features
- macOS for hardware security module support (optional)

### Building

```bash
mkdir build
cd build
cmake ..
make
```

This will build:
- Static library (`libhydra_vfs.a`)
- Shared library (`libhydra_vfs.so` on Linux, `libhydra_vfs.dylib` on macOS, `hydra_vfs.dll` on Windows)
- Demo executable (`hydra_sks`)
- Test executable (`vfs_tests`)

### Testing

Run the test suite to verify everything is working correctly:

```bash
./vfs_tests
```

The tests include:
- Basic file operations (read/write)
- Directory operations (create, list, delete)
- Encrypted file operations
- Container security verification
- Cross-VFS mounting

### Running the Demo

```bash
./hydra_sks
```

## API Overview

### Creating a VFS

```cpp
// Create an in-memory VFS
auto memory_vfs = hydra::vfs::create_memory_vfs();

// Create a persistent VFS with a root directory
auto persistent_vfs = hydra::vfs::create_persistent_vfs("./vfs_data");

// Create an encrypted VFS
auto encryption_provider = std::make_shared<hydra::vfs::AESEncryptionProvider>();
auto encryption_key = hydra::vfs::EncryptionKey::generate(256);
auto encrypted_vfs = hydra::vfs::create_encrypted_vfs(persistent_vfs, encryption_provider, encryption_key);

// Create a container VFS with resource limits
hydra::vfs::ResourceLimits limits;
limits.max_files = 1000;
limits.max_directories = 100;
limits.max_storage_bytes = 1024 * 1024 * 10; // 10 MB

auto container_vfs = hydra::vfs::create_container_vfs(
    "./container.dat",
    encryption_provider,
    encryption_key,
    persistent_vfs,
    hydra::vfs::SecurityLevel::HARDWARE_BACKED,
    limits
);
```

### File Operations

```cpp
// Create a file
vfs->create_file("/path/to/file.txt");

// Open a file
auto file = vfs->open_file("/path/to/file.txt", hydra::vfs::FileMode::WRITE);

// Write to a file
file->write(data, size);

// Read from a file
file->read(buffer, size);

// Close a file
file->close();
```

### Directory Operations

```cpp
// Create a directory
vfs->create_directory("/path/to/dir");

// List a directory
auto entries = vfs->list_directory("/path/to/dir");

// Check if a directory exists
bool exists = vfs->directory_exists("/path/to/dir");

// Delete a directory
vfs->delete_directory("/path/to/dir", recursive);
```

### Mount Operations

```cpp
// Mount one VFS inside another
parent_vfs->mount("/mount/point", child_vfs);

// Unmount a VFS
parent_vfs->unmount("/mount/point");
```

## Security Features

- **Isolation**: The VFS is completely isolated from the internet
- **Path Normalization**: All paths are normalized to prevent path traversal attacks
- **Controlled Access**: All file operations go through the VFS API, which can enforce access controls
- **No Network Access**: The VFS has no network capabilities, making it suitable for sensitive data
- **Encryption**: Files can be transparently encrypted using AES-256
- **Hardware Security**: Container VFS can use hardware security modules for enhanced protection
- **Integrity Verification**: Container VFS includes integrity verification to detect tampering
- **Resource Limits**: Container VFS can enforce limits on storage, files, and directories

## Container VFS

The Container VFS provides a Docker-like isolated environment with hardware-level security features:

- **Single File Container**: All data is stored in a single encrypted container file
- **Hardware-Backed Security**: Uses platform-specific security hardware when available (e.g., macOS Secure Enclave)
- **Resource Monitoring**: Enforces limits on storage, file count, and directory count
- **Integrity Verification**: Verifies the integrity of the container and its contents
- **Transparent Encryption**: All data is encrypted using AES-256 or hardware security
- **Robust Error Handling**: Automatically recovers from corruption by recreating files when necessary
- **Parent Directory Creation**: Automatically creates parent directories when needed for container files

## Using Shared Libraries

The project builds both static and shared libraries to support various integration scenarios:

### Linux (.so)
```cpp
// Link with -lhydra_vfs
#include <hydra_vfs/vfs.h>
```

### macOS (.dylib)
```cpp
// Link with -lhydra_vfs
#include <hydra_vfs/vfs.h>
```

### Windows (.dll)
```cpp
// Link with hydra_vfs.lib and ensure hydra_vfs.dll is in the PATH
#include <hydra_vfs/vfs.h>
```

## License

This project is available under the MIT License.
