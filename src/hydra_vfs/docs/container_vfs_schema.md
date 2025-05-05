# Container VFS Schema

This document describes the schema and structure of the Container VFS implementation, which provides a Docker-like isolated environment with hardware-level security features.

## Container File Structure

The Container VFS stores all data in a single encrypted file with the following structure:

```
+----------------------------------+
| Container Header                 |
|  - Magic Number                  |
|  - Version                       |
|  - Security Level                |
|  - Metadata Offset               |
|  - Metadata Size                 |
|  - Container Metadata Offset     |
|  - Container Metadata Size       |
|  - Data Offset                   |
|  - Data Size                     |
+----------------------------------+
| Encrypted Metadata               |
|  - Directory Structure           |
|  - File Entries                  |
|  - Integrity Hashes              |
+----------------------------------+
| Encrypted Container Metadata     |
|  - Container ID                  |
|  - Creator                       |
|  - Creation Time                 |
|  - Last Modified Time            |
|  - Integrity Hash                |
+----------------------------------+
| Encrypted File Data              |
|  - File 1 Content                |
|  - File 2 Content                |
|  - ...                           |
+----------------------------------+
```

## Class Hierarchy

```
+----------------------+     +----------------------+
| IVirtualFileSystem   |     | IFile                |
+----------------------+     +----------------------+
          ^                            ^
          |                            |
+----------------------+     +----------------------+
| ContainerVFS         |<--->| ContainerFile       |
+----------------------+     +----------------------+
          |
          v
+----------------------+     +----------------------+
| ResourceMonitor      |     | IHardwareSecurityModule
+----------------------+     +----------------------+
                                        ^
                                        |
                              +----------------------+
                              | MacOSSecurityModule  |
                              +----------------------+
```

## Data Structures

### ContainerHeader

```cpp
struct ContainerHeader {
    static constexpr uint32_t MAGIC = 0x48595652; // "HYDRA"
    static constexpr uint32_t VERSION = 1;
    
    uint32_t magic;
    uint32_t version;
    uint32_t security_level;
    uint64_t metadata_offset;
    uint64_t metadata_size;
    uint64_t container_metadata_offset;
    uint64_t container_metadata_size;
    uint64_t data_offset;
    uint64_t data_size;
};
```

### ContainerEntry

```cpp
struct ContainerEntry {
    enum Type {
        FILE,
        DIRECTORY
    };
    
    Type type;
    std::string name;
    uint64_t timestamp;
    std::weak_ptr<ContainerEntry> parent;
    std::vector<std::shared_ptr<ContainerEntry>> children;
    
    // For files only
    uint64_t size;
    uint64_t data_offset;
    std::vector<uint8_t> integrity_hash;
};
```

### ContainerMetadata

```cpp
struct ContainerMetadata {
    std::array<uint8_t, 16> container_id;
    std::array<char, 32> creator;
    uint64_t creation_time;
    uint64_t last_modified_time;
    std::vector<uint8_t> integrity_hash;
};
```

### ResourceLimits

```cpp
struct ResourceLimits {
    size_t max_files;
    size_t max_directories;
    size_t max_storage_bytes;
};
```

## Security Levels

```cpp
enum class SecurityLevel {
    STANDARD,       // Software-based encryption
    HARDWARE_BACKED // Hardware security module when available
};
```

## Flow Diagrams

### Container Initialization

```
+----------------+     +----------------+     +----------------+
| Create         |     | Initialize     |     | Create Root    |
| Container VFS  |---->| Container      |---->| Directory      |
+----------------+     +----------------+     +----------------+
                              |
                              v
+----------------+     +----------------+
| Save           |<----| Initialize     |
| Metadata       |     | Hardware       |
+----------------+     | Security       |
                       +----------------+
```

### File Operations

```
+----------------+     +----------------+     +----------------+
| Open File      |     | Get Entry      |     | Create         |
| Request        |---->| from Path      |---->| ContainerFile  |
+----------------+     +----------------+     +----------------+
                                                      |
                                                      v
+----------------+     +----------------+     +----------------+
| Encrypt/       |<----| Read/Write     |<----| Check Resource |
| Decrypt Data   |     | Operation      |     | Limits         |
+----------------+     +----------------+     +----------------+
        |
        v
+----------------+     +----------------+
| Calculate      |---->| Update         |
| Integrity Hash |     | Container      |
+----------------+     | Metadata       |
                       +----------------+
```

## Resource Monitoring

The ResourceMonitor tracks and enforces limits on:

1. Number of files
2. Number of directories
3. Total storage usage in bytes

When a resource limit would be exceeded, operations return an `ErrorCode::RESOURCE_LIMIT_EXCEEDED` error.

## Integrity Verification

For containers with hardware-backed security:

1. Each file has an integrity hash calculated using the hardware security module
2. The container has a master integrity hash that combines all file hashes
3. Integrity is verified when loading the container and when reading files

## Hardware Security Integration

When hardware security is available:

1. Encryption keys are stored in the hardware security module
2. Encryption/decryption operations are performed by the hardware
3. Integrity hashes are calculated using hardware-accelerated functions

On platforms without hardware security, the system falls back to software-based encryption while maintaining the same API.
