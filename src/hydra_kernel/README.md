# Hydra Kernel Library

## Overview

The `hydra_kernel` library is a powerful virtual process management system for the Hydra SDK. It provides a secure, isolated environment for running processes, managing inter-process communication, and controlling network access. The kernel acts as a containerization layer, similar to Docker but with a focus on security and isolation.

## Key Components

### Process Management

- **HydraKernel**: The core component that manages virtual processes, providing process creation, termination, and monitoring capabilities.
- **Process**: Represents a virtual process within the kernel, with its own isolated environment, resource limits, and execution context.
- **ProcessLimits**: Configurable resource limits for processes, including memory, CPU, I/O operations, file descriptors, and thread count.
- **ProcessStats**: Runtime statistics for processes, including resource usage and execution time.

### Network Virtualization

- **PortForwarder**: Manages port forwarding between the virtual environment and the host system, allowing controlled network access.
- **NetworkInterface**: Represents a virtual network interface with configurable IP address, netmask, and MTU.
- **Connection**: Manages individual network connections, tracking statistics and handling data transfer.

### Inter-Process Communication (IPC)

- **MessageQueue**: Provides a queue-based messaging system for communication between processes.
- **SharedMemory**: Enables processes to share memory segments for efficient data exchange.
- **Semaphore**: Provides synchronization primitives for coordinating access to shared resources.
- **IPCManager**: Central manager for creating and managing IPC objects.

## Usage Examples

### Creating and Managing Processes

```cpp
#include "hydra_kernel/kernel.h"
#include "hydra_vfs/vfs.h"
#include <iostream>

using namespace hydra::kernel;
using namespace hydra::vfs;

int main() {
    // Create a virtual file system
    auto vfs = create_vfs("/tmp/hydra_kernel_demo");
    
    // Create the kernel with the VFS
    HydraKernel kernel(vfs);
    
    // Start the kernel
    if (!kernel.start()) {
        std::cerr << "Failed to start kernel" << std::endl;
        return 1;
    }
    
    // Create a process
    auto process = kernel.createProcess("demo_process");
    if (!process) {
        std::cerr << "Failed to create process" << std::endl;
        return 1;
    }
    
    // Set resource limits for the process
    ProcessLimits limits;
    limits.memory_limit = 100 * 1024 * 1024; // 100 MB
    limits.cpu_limit = 0.5f; // 50% CPU
    process->setLimits(limits);
    
    // Execute a function in the process
    int result = process->executeFunction([]() -> int {
        // This code runs in the isolated process
        std::cout << "Hello from isolated process!" << std::endl;
        return 0;
    });
    
    // Get process statistics
    ProcessStats stats = process->getStats();
    std::cout << "Process memory usage: " << stats.memory_usage << " bytes" << std::endl;
    
    // Terminate the process
    process->terminate(0);
    
    // Stop the kernel
    kernel.stop();
    
    return 0;
}
```

### Inter-Process Communication

```cpp
#include "hydra_kernel/ipc.h"
#include <iostream>
#include <vector>
#include <string>

using namespace hydra::kernel;

// Process 1: Sender
void senderProcess() {
    // Get a message queue
    auto queue = IPCManager::getMessageQueue("example_queue");
    
    // Send a message
    std::string message = "Hello from sender process!";
    std::vector<uint8_t> data(message.begin(), message.end());
    queue->send(data);
    
    // Create shared memory
    auto shm = IPCManager::getSharedMemory("example_memory", 1024);
    
    // Lock the shared memory for writing
    shm->lock();
    
    // Write data to shared memory
    uint8_t* mem = shm->getData();
    std::string shm_message = "Data in shared memory";
    std::copy(shm_message.begin(), shm_message.end(), mem);
    
    // Unlock the shared memory
    shm->unlock();
    
    // Signal that data is ready
    auto sem = IPCManager::getSemaphore("example_semaphore");
    sem->signal();
}

// Process 2: Receiver
void receiverProcess() {
    // Get the same message queue
    auto queue = IPCManager::getMessageQueue("example_queue");
    
    // Receive a message (blocking)
    auto received = queue->receive(true);
    if (received) {
        std::string message(received->begin(), received->end());
        std::cout << "Received message: " << message << std::endl;
    }
    
    // Wait for shared memory data to be ready
    auto sem = IPCManager::getSemaphore("example_semaphore");
    sem->wait();
    
    // Get the shared memory
    auto shm = IPCManager::getSharedMemory("example_memory", 1024);
    
    // Lock the shared memory for reading
    shm->lock();
    
    // Read data from shared memory
    const uint8_t* mem = shm->getData();
    std::string shm_message;
    for (size_t i = 0; i < shm->getSize() && mem[i] != 0; ++i) {
        shm_message.push_back(static_cast<char>(mem[i]));
    }
    
    // Unlock the shared memory
    shm->unlock();
    
    std::cout << "Received from shared memory: " << shm_message << std::endl;
}
```

### Network Virtualization

```cpp
#include "hydra_kernel/kernel.h"
#include "hydra_kernel/network.h"
#include <iostream>

using namespace hydra::kernel;

int main() {
    // Create a kernel
    auto vfs = hydra::vfs::create_vfs("/tmp/hydra_kernel_network_demo");
    HydraKernel kernel(vfs);
    kernel.start();
    
    // Create a port forwarding rule
    kernel.createPort(8080, 8080, "tcp");
    
    // Create a process that will handle connections
    auto server_process = kernel.createProcess("server_process");
    
    // Bind the process to the port
    kernel.bindProcessToPort(server_process->getPID(), 8080);
    
    // In the server process, set up a handler for incoming connections
    server_process->executeFunction([]() -> int {
        // Set up a data handler for incoming connections
        auto handler = [](const uint8_t* data, size_t size) {
            std::string message(reinterpret_cast<const char*>(data), size);
            std::cout << "Received: " << message << std::endl;
            
            // Echo back
            std::string response = "Echo: " + message;
            // Send response back (implementation depends on the specific API)
        };
        
        // In a real implementation, you would register this handler with the kernel
        
        // Keep the server running
        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        
        return 0;
    });
    
    // Create a client process
    auto client_process = kernel.createProcess("client_process");
    
    // Connect to the port
    kernel.connectToPort(client_process->getPID(), 8080);
    
    // Clean up
    kernel.terminateProcess(server_process->getPID());
    kernel.terminateProcess(client_process->getPID());
    kernel.closePort(8080);
    kernel.stop();
    
    return 0;
}
```

## Security Features

- **Process Isolation**: Each process runs in its own isolated environment with controlled access to resources.
- **Resource Limits**: Configurable limits on memory, CPU, I/O operations, file descriptors, and thread count.
- **Network Isolation**: Controlled network access through port forwarding and virtual network interfaces.
- **File System Isolation**: Each process can have its own isolated file system or share with others.
- **Isolation Modes**: Configurable isolation levels (none, partial, complete) to balance security and functionality.

## Integration with Hydra SDK

The `hydra_kernel` library integrates with other components of the Hydra SDK:

- **hydra_vfs**: Provides the isolated file system for processes.
- **hydra_crypto**: Secures communication channels and data storage.
- **hydra_math**: Provides mathematical operations for cryptographic algorithms.
- **hydra_server**: Builds secure services on top of the kernel's process management.

## Dependencies

- C++17 compatible compiler
- POSIX-compliant operating system (Linux, macOS)
- Threading support
- Network socket API

## Building

```bash
cd src/hydra_kernel
mkdir build && cd build
cmake ..
make
```

## Future Developments

- Enhanced security features for process isolation
- Support for hardware-level virtualization
- Performance optimizations for resource-constrained environments
- Extended API for more fine-grained control over process execution
