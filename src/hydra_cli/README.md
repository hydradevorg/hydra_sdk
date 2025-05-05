# Hydra CLI Tool

The Hydra CLI tool provides command-line access to Hydra SDK functionality, enabling secure cryptographic operations and virtual file system management through a simple interface.

## Building

The Hydra CLI is built as part of the Hydra SDK. From the root of the SDK:

```bash
mkdir -p build && cd build
cmake ..
make
```

The binary will be located at `build/bin/hydra-cli`.

## Commands

The Hydra CLI tool is organized into command groups:

### Crypto Commands

Cryptographic operations for secure key management, encryption, decryption, signing, and verification.

#### Key Management

```bash
# Generate a key pair
hydra-cli crypto keygen --type dilithium --output-public public_key.bin --output-private private_key.bin

# Display key information
hydra-cli crypto keyinfo --key public_key.bin
```

#### Encryption & Decryption

```bash
# Encrypt a file using a public key
hydra-cli crypto encrypt --key public_key.bin --input plaintext.txt --output encrypted.bin

# Decrypt a file using a private key
hydra-cli crypto decrypt --key private_key.bin --input encrypted.bin --output decrypted.txt
```

#### Signing & Verification

```bash
# Sign a file using a private key
hydra-cli crypto sign --key private_key.bin --input document.txt --output signature.bin

# Verify a signature against a file
hydra-cli crypto verify --key public_key.bin --input document.txt --signature signature.bin
```

### VFS Commands

Virtual File System operations for secure file storage and management.

#### Container Management

```bash
# Create a new VFS container
hydra-cli vfs container create --password mypassword /path/to/container.vfs

# View information about a container
hydra-cli vfs container info --password mypassword /path/to/container.vfs
```

#### File Operations

```bash
# List files in a container
hydra-cli vfs ls --password mypassword /path/to/container.vfs /

# Display the contents of a file in a container
hydra-cli vfs cat --password mypassword /path/to/container.vfs /path/to/file.txt

# Extract a file from a container to the local filesystem
hydra-cli vfs get --password mypassword --output local_file.txt /path/to/container.vfs /path/to/file.txt

# Add a file to a container
hydra-cli vfs put --password mypassword --input local_file.txt /path/to/container.vfs /path/to/file.txt

# Remove a file from a container
hydra-cli vfs rm --password mypassword /path/to/container.vfs /path/to/file.txt

# Create a directory in a container
hydra-cli vfs mkdir --password mypassword /path/to/container.vfs /path/to/directory
```

## Authentication Options

For secure operations, the Hydra CLI supports multiple authentication methods:

### Password Authentication

Use `--password` followed by your password:

```bash
hydra-cli vfs cat --password mypassword /path/to/container.vfs /path/to/file.txt
```

### Key File Authentication

Use `--key-file` followed by the path to your key file:

```bash
hydra-cli vfs cat --key-file /path/to/key.bin /path/to/container.vfs /path/to/file.txt
```

## Security Considerations

- Store key files securely and consider using access controls
- Use strong passwords for container encryption
- Be mindful of terminal history which may store commands with passwords
- Consider using key files instead of passwords for automated scripts

## Examples

### Creating and Using an Encrypted Container

```bash
# Create a new container
hydra-cli vfs container create --password securepassword ~/secure_container.vfs

# Add a file to the container
hydra-cli vfs put --password securepassword --input sensitive_document.txt ~/secure_container.vfs /secret.txt

# View the file contents
hydra-cli vfs cat --password securepassword ~/secure_container.vfs /secret.txt

# Extract the file to a different location
hydra-cli vfs get --password securepassword --output extracted_secret.txt ~/secure_container.vfs /secret.txt
```

### Signing and Verifying a Document

```bash
# Generate a key pair
hydra-cli crypto keygen --type dilithium --output-public verification_key.bin --output-private signing_key.bin

# Sign a document
hydra-cli crypto sign --key signing_key.bin --input important_document.pdf --output document_signature.bin

# Verify the signature
hydra-cli crypto verify --key verification_key.bin --input important_document.pdf --signature document_signature.bin
```

## Troubleshooting

If you encounter issues:

1. Ensure you're using the correct key or password
2. Check file paths and permissions
3. For VFS operations, verify the container exists and is not corrupted
4. Consult debug logs with the `--verbose` flag for detailed information
