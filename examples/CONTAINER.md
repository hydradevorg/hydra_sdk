# Hydra SDK Docker-like Container Guide

This guide explains how to use the new Docker-like container configuration features in Hydra SDK. This approach makes it much easier to create and manage secure encrypted containers with advanced features like automatic key generation and directory mounting.

## Container Configuration Format

Hydra containers can now be defined using YAML configuration files, similar to Docker Compose. Here's an example:

```yaml
version: '1.0'

container:
  name: secure_vault
  path: ./secure_vault.hcon

security:
  encryption: kyber_aes
  security_level: standard

resources:
  max_storage: 1GB
  max_files: 10000
  max_file_size: 100MB

mounts:
  - source: ./data
    target: /imported_data
    read_only: false
```

## Quick Start

### 1. Generate a Configuration File

```bash
# Generate a default configuration file
hydra-cli vfs container init my_container
```

This creates a file named `my_container.yml` with default settings that you can customize.

### 2. Create a Container

```bash
# Create a container from the configuration file
hydra-cli vfs container create my_container.yml
```

If no password or key file is specified, the command will automatically generate a post-quantum secure Kyber key pair and store the private key in a `.key` file next to your container.

Alternatively, you can specify a password:

```bash
# Create a container with a specific password
hydra-cli vfs container create my_container.yml --password your_password_here
```

### 3. Access the Container

```bash
# View container information
hydra-cli vfs container info ./my_container.hcon --password your_password_here
# or
hydra-cli vfs container info ./my_container.hcon --key-file ./my_container.key
```

## Configuration Options

### Container Section

- `name`: A human-readable name for the container
- `path`: The path where the container file will be created

### Security Section

- `encryption`: Encryption algorithm to use (`aes256` or `kyber_aes`)
- `security_level`: Security level (`standard` or `hardware_backed`)
- `password`: Optional password for encryption (not recommended for production use)
- `key_file`: Optional path to a key file

### Resources Section

- `max_storage`: Maximum total storage size (e.g., `1GB`, `500MB`)
- `max_files`: Maximum number of files allowed in the container
- `max_file_size`: Maximum size for a single file

### Mounts Section

Mounts allow you to import files from the host system into the container during creation:

```yaml
mounts:
  - source: ./local_directory
    target: /container_directory
    read_only: false
```

- `source`: Path to a directory on the host system
- `target`: Path where the directory should be mounted in the container
- `read_only`: Whether the mount should be read-only (currently not enforced)

## Post-Quantum Security

The Hydra container system supports post-quantum secure encryption using Kyber KEM. When using the `kyber_aes` encryption option without specifying a password or key file, the system will:

1. Generate a Kyber key pair
2. Save the private key to a `.key` file
3. Use the private key for container encryption

This provides a future-proof security model that can resist attacks from quantum computers.

## Directory Mounting

The mounts configuration allows you to automatically import files from your host system into the container. This works similar to Docker volumes but as a one-time import during container creation.

## Legacy Mode

For backward compatibility, the legacy container creation command is still supported:

```bash
hydra-cli vfs container create ./my_container.hcon --password your_password_here
```

However, the Docker-like configuration approach is recommended for new projects.
