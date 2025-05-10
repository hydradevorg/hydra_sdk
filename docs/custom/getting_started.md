# Getting Started with Hydra SDK

This guide will help you get started with the Hydra SDK, covering basic usage and common tasks.

## First Steps

After [installing](installation.md) the Hydra SDK, you can start exploring its functionality using the Hydra CLI.

### Checking the Version

```bash
hydra-cli --version
```

### Getting Help

```bash
hydra-cli --help
```

## Basic Cryptographic Operations

The Hydra SDK provides a range of cryptographic operations through the `crypto` category of commands.

### Generating Keys

```bash
# Generate a Kyber key pair
hydra-cli crypto keygen --algorithm kyber --type private private_key.bin
hydra-cli crypto keygen --algorithm kyber --type public public_key.bin

# Generate a Falcon key pair
hydra-cli crypto keygen --algorithm falcon --type private private_key.bin
hydra-cli crypto keygen --algorithm falcon --type public public_key.bin

# Generate a Dilithium key pair
hydra-cli crypto keygen --algorithm dilithium --type private private_key.bin
hydra-cli crypto keygen --algorithm dilithium --type public public_key.bin
```

### Encrypting and Decrypting Data

```bash
# Encrypt a file
hydra-cli crypto encrypt --algorithm kyber --key public_key.bin input.txt output.enc

# Decrypt a file
hydra-cli crypto decrypt --algorithm kyber --key private_key.bin output.enc decrypted.txt
```

### Signing and Verifying Data

```bash
# Sign a file
hydra-cli crypto sign --algorithm falcon --key private_key.bin input.txt signature.bin

# Verify a signature
hydra-cli crypto verify --algorithm falcon --key public_key.bin input.txt signature.bin
```

### Generating Addresses

```bash
# Generate a standard address
hydra-cli crypto address standard --type user public_key.bin

# Generate a geohashed address
hydra-cli crypto address geo --type node 37.7749 -122.4194 public_key.bin

# Generate a compressed address
hydra-cli crypto address compressed --type resource public_key.bin

# Verify an address
hydra-cli crypto address verify <ADDRESS>
```

## Working with the Virtual File System

The Hydra SDK includes a secure Virtual File System (VFS) for storing and retrieving data.

### Creating a Container

```bash
# Create a new encrypted container
hydra-cli vfs container create --password mysecretpassword mycontainer.vfs
```

### Adding Files

```bash
# Add a file to the container
hydra-cli vfs put --password mysecretpassword mycontainer.vfs local_file.txt /remote/path/file.txt
```

### Listing Files

```bash
# List files in the container
hydra-cli vfs ls --password mysecretpassword mycontainer.vfs /remote/path
```

### Retrieving Files

```bash
# Get a file from the container
hydra-cli vfs get --password mysecretpassword mycontainer.vfs /remote/path/file.txt local_copy.txt
```

### Removing Files

```bash
# Remove a file from the container
hydra-cli vfs rm --password mysecretpassword mycontainer.vfs /remote/path/file.txt
```

## Working with Containers

The Hydra SDK provides container isolation for secure execution environments.

### Running a Container

```bash
# Run a container with a specific configuration
hydra-cli kernel run --config container_config.yaml
```

### Listing Containers

```bash
# List all running containers
hydra-cli kernel list
```

### Executing Commands in a Container

```bash
# Execute a command in a running container
hydra-cli kernel exec --id container_id command
```

### Stopping a Container

```bash
# Stop a running container
hydra-cli kernel stop --id container_id
```

## Next Steps

Now that you're familiar with the basic functionality of the Hydra SDK, you can:

1. Explore the [API Reference](api_reference.md) for detailed information about each module
2. Check out the [Examples](examples.md) for more complex usage scenarios
3. Learn about [Advanced Usage](advanced_usage.md) for advanced features and techniques

For more information, refer to the documentation for each module:

- [Hydra CLI](../modules/hydra_cli.html)
- [Hydra Crypto](../modules/hydra_crypto.html)
- [Hydra VFS](../modules/hydra_vfs.html)
- [Hydra Address](../modules/hydra_address.html)
- [Hydra QZKP](../modules/hydra_qzkp.html)
- [Hydra Kernel](../modules/hydra_kernel.html)
- [Hydra LMVS](../modules/hydra_lmvs.html)
- [Hydra Math](../modules/hydra_math.html)
- [Hydra Compression](../modules/hydra_compression.html)
