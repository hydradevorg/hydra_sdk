# Hydra Mini-Kernel Example

This example demonstrates how to use the Hydra Kernel module to create a lightweight process isolation layer with Docker-like container features.

## Key Features

### Process Isolation

The Hydra Kernel provides process isolation with:

- **Virtual processes** - Lightweight process abstraction
- **Resource limits** - Control memory, CPU, and file usage
- **Virtual file systems** - Each process can have its own isolated filesystem
- **IPC mechanisms** - Secure communication between processes

### Network Isolation

The kernel supports three isolation modes:

1. **No Isolation** (Mode 0) - Processes have direct access to the host network
2. **Partial Isolation** (Mode 1) - Processes can only access the network through port forwarding
3. **Complete Isolation** (Mode 2) - Processes have no network access

### Port Forwarding

Similar to Docker port mapping, the kernel supports:

- Mapping internal container ports to external host ports
- Protocol selection (TCP/UDP)
- Port binding to specific processes
- Connection monitoring and statistics

## Example Flow

This example demonstrates:

1. Creating a secure container VFS using the Hydra VFS
2. Initializing a mini-kernel with the container VFS
3. Setting up process isolation and port forwarding
4. Creating multiple processes with different roles:
   - Server process - Handles IPC requests
   - Client process - Sends requests to the server
   - Web server process - Listens on a port with port forwarding
5. Inter-process communication using message queues
6. Network communication through port forwarding
7. Resource monitoring and statistics

## Usage

To run the example:

```bash
# Build the project
mkdir build
cd build
cmake ..
make

# Run the example
./bin/kernel_example
```

## Docker-like Configuration

The Hydra Kernel can be configured using a YAML configuration file similar to Docker Compose:

```yaml
version: '1.0'

container:
  name: my_application
  path: ./app_container.hcon

security:
  encryption: kyber_aes
  security_level: standard

isolation:
  mode: partial  # none, partial, complete
  ports:
    - internal: 8080
      external: 80
      protocol: tcp
      process: web_server

resources:
  max_memory: 1GB
  max_cpu: 0.5  # 50% CPU limit
  max_files: 1000
  max_file_size: 100MB

processes:
  - name: web_server
    command: /bin/web_server
    env:
      PORT: 8080
    limits:
      memory: 512MB
      cpu: 0.3
  
  - name: worker
    command: /bin/worker
    count: 4
    env:
      WORKER_MODE: background
```

## Extending the Example

You can extend this example in several ways:

1. Add more advanced IPC mechanisms like shared memory
2. Implement process scheduling with priorities
3. Create a network of virtual processes that communicate with each other
4. Implement file system mounting between processes
5. Add support for containerized applications with automatic isolation

## Security Considerations

The Hydra Kernel provides isolation through a combination of:

1. **File system isolation** - Processes only see their own VFS
2. **Network isolation** - Controlled access to the network
3. **IPC restrictions** - Processes communicate only through explicit channels
4. **Resource limits** - Prevent resource exhaustion attacks

For production use, you might want to enhance security with:

1. Mandatory access controls
2. Hardware-backed security (TPM/Secure Enclave)
3. Formal verification of isolation boundaries

## Technical Implementation

The kernel implementation includes:

- `Process` class - Virtual process abstraction
- `HydraKernel` class - Process management and resource tracking
- `PortForwarder` class - Network isolation and port mapping
- `IPCManager` class - Inter-process communication facilities

Each component is designed to be modular and extensible, allowing you to tailor the isolation level to your specific needs.
