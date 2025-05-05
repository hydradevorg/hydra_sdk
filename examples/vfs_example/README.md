# Hydra VFS Integration Examples

This directory contains example projects that demonstrate how to integrate and use the Hydra VFS library in your applications.

## Examples

### 1. Simple Examples (`simple_example.cpp`)

This example demonstrates the basic usage of Hydra VFS, including:
- Creating memory, persistent, and container VFS instances
- Basic file and directory operations
- Reading and writing encrypted data
- Working with different VFS types

### 2. Secure File Manager (`secure_file_manager.cpp`)

A complete application example that implements a secure file manager with the following features:
- Password-based container encryption
- Interactive command-line interface
- File system commands: ls, cd, mkdir, rm, etc.
- File viewing, editing, and manipulation
- Import/export between VFS and regular file system
- Binary file support with hex dump viewing

## Building the Examples

### Prerequisites

- C++20 compatible compiler
- CMake 3.10 or higher
- Compiled Hydra VFS library (static or shared)
- OpenSSL development libraries

### Build Instructions

1. Make sure you've already built the Hydra VFS library from the parent directory
2. Build the examples:

```bash
mkdir build
cd build
cmake ..
make
```

This will produce two executables:
- `simple_example`: Basic usage examples of Hydra VFS
- `secure_file_manager`: Interactive secure file manager application

## Running the Examples

### Simple Example

```bash
./simple_example
```

This will:
1. Create an in-memory VFS and demonstrate basic operations
2. Create a persistent VFS in `./vfs_data` and store files
3. Create an encrypted container in `./container.dat` with sample encrypted content

### Secure File Manager

```bash
./secure_file_manager [container_path]
```

Where `container_path` is optional and defaults to `./secure_container.dat`.

The application will prompt for a password to create or unlock the container. After that, you'll be presented with an interactive shell to work with the secure file system.

Type `help` in the application to see all available commands:
- `ls`, `dir`: List directory contents
- `cd`: Change current directory
- `mkdir`: Create new directory
- `rm`, `del`: Remove files or directories
- `cat`, `type`: View file contents
- `write`: Write text to a file
- `import`: Import a file from the external file system
- `export`: Export a file to the external file system
- `exit`, `quit`: Exit the application

## Integration Tips

1. **Linking with the Hydra VFS library**:
   - For static library: Link with `-lhydra_vfs`
   - For shared library: Ensure the library is in your library path

2. **Header Includes**:
   ```cpp
   #include <hydra_vfs/vfs.h>
   #include <hydra_vfs/memory_vfs.h>
   #include <hydra_vfs/persistent_vfs.h>
   #include <hydra_vfs/container_vfs.h>
   ```

3. **Error Handling**:
   - All operations return a `Result` object which you should check with `.success()`
   - Get error messages with `.error()`
   - Get result values with `.value()`

4. **Platform-Specific Considerations**:
   - On Windows, ensure the .dll is in your PATH or the same directory as your executable
   - On macOS, hardware security features may require additional permissions
   - On Linux, shared objects (.so) should be in your library path

5. **Security Best Practices**:
   - Use proper key derivation functions (PBKDF2, Argon2) in production code
   - Implement proper access controls around your container files
   - Clear sensitive data from memory when no longer needed
