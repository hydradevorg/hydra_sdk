#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <chrono>
#include <hydra_vfs/vfs.h>
#include <hydra_vfs/container_vfs.h>
#include <hydra_kernel/kernel.h>
#include <hydra_kernel/process.h>
#include <hydra_kernel/network.h>
#include <hydra_kernel/ipc.h>

using namespace hydra;

// Server process function
int server_process(int port) {
    std::cout << "Server process started on port " << port << std::endl;
    
    // Create a message queue for receiving requests
    auto request_queue = kernel::IPCManager::getMessageQueue("server_requests");
    
    // Create a message queue for sending responses
    auto response_queue = kernel::IPCManager::getMessageQueue("server_responses");
    
    // Process requests
    int request_count = 0;
    bool running = true;
    
    while (running) {
        // Check for requests
        auto request_opt = request_queue->receive(true, 1000); // Wait for 1 second
        
        if (request_opt) {
            // Got a request
            auto request = request_opt.value();
            
            // Convert request to string
            std::string request_str(request.begin(), request.end());
            std::cout << "Server received request: " << request_str << std::endl;
            
            // Process request
            std::string response;
            
            if (request_str == "exit") {
                // Exit command
                response = "Server shutting down";
                running = false;
            } else if (request_str == "ping") {
                // Ping command
                response = "pong";
            } else if (request_str == "time") {
                // Time command
                auto now = std::chrono::system_clock::now();
                auto now_time = std::chrono::system_clock::to_time_t(now);
                response = std::ctime(&now_time);
            } else if (request_str == "count") {
                // Count command
                response = "Request count: " + std::to_string(request_count);
            } else {
                // Unknown command
                response = "Unknown command: " + request_str;
            }
            
            // Send response
            std::vector<uint8_t> response_data(response.begin(), response.end());
            response_queue->send(response_data);
            
            request_count++;
        }
    }
    
    std::cout << "Server process stopped" << std::endl;
    return 0;
}

// Client process function
int client_process(const std::vector<std::string>& commands) {
    std::cout << "Client process started" << std::endl;
    
    // Create a message queue for sending requests
    auto request_queue = kernel::IPCManager::getMessageQueue("server_requests");
    
    // Create a message queue for receiving responses
    auto response_queue = kernel::IPCManager::getMessageQueue("server_responses");
    
    // Send requests
    for (const auto& command : commands) {
        std::cout << "Client sending command: " << command << std::endl;
        
        // Send request
        std::vector<uint8_t> request_data(command.begin(), command.end());
        request_queue->send(request_data);
        
        // Wait for response
        auto response_opt = response_queue->receive(true, 5000); // Wait for 5 seconds
        
        if (response_opt) {
            // Got a response
            auto response = response_opt.value();
            
            // Convert response to string
            std::string response_str(response.begin(), response.end());
            std::cout << "Client received response: " << response_str << std::endl;
        } else {
            std::cerr << "Client timed out waiting for response" << std::endl;
        }
        
        // Sleep for a bit
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    std::cout << "Client process finished" << std::endl;
    return 0;
}

// Web server process function with port forwarding
int web_server_process(int port) {
    std::cout << "Web server process started on port " << port << std::endl;
    
    // Process is running in a container, but it can receive requests from the host
    // via the port forwarding mechanism
    
    // Simulate a web server
    int request_count = 0;
    bool running = true;
    
    // Get the kernel
    auto& kernel = kernel::getKernel();
    
    // Create port
    kernel.createPort(port, port, "tcp");
    
    // Create a process
    auto process = kernel.createProcess("server_process");
    
    // Bind process to port
    kernel.bindProcessToPort(process->getPID(), port);
    
    // Since we don't have direct access to the port forwarder,
    // we'll use a different approach for handling network data
    
    // Execute a function in the process to handle network connections
    process->executeFunction([&]() -> int {
        std::cout << "Server process started, listening on port " << port << std::endl;
        
        // Simulate handling network data
        std::cout << "Waiting for connections..." << std::endl;
        
        // Simulate receiving a request
        std::string request_str = "GET /ping HTTP/1.1\r\nHost: localhost\r\n\r\n";
        std::cout << "Web server received request: " << request_str << std::endl;
        
        // Process request
        std::string response;
        
        if (request_str.find("GET /exit") != std::string::npos) {
            // Exit command
            response = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\nServer shutting down";
            running = false;
        } else if (request_str.find("GET /ping") != std::string::npos) {
            // Ping command
            response = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\npong";
        } else if (request_str.find("GET /time") != std::string::npos) {
            // Time command
            auto now = std::chrono::system_clock::now();
            auto now_time = std::chrono::system_clock::to_time_t(now);
            response = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\n";
            response += std::ctime(&now_time);
        } else if (request_str.find("GET /count") != std::string::npos) {
            // Count command
            response = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\n";
            response += "Request count: " + std::to_string(request_count);
        } else {
            // Unknown command
            response = "HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\n\r\n";
            response += "Unknown command: " + request_str;
        }
        
        // Send response back through the connection
        // In a real implementation, we would send the response to the correct connection
        // For demonstration purposes, we'll just print it
        std::cout << "Web server sending response: " << response << std::endl;
        
        request_count++;
    });
    
    // Run until stopped
    while (running) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    
    // Close port
    kernel.closePort(port);
    
    std::cout << "Web server process stopped" << std::endl;
    return 0;
}

int main(int argc, char** argv) {
    std::cout << "Hydra Kernel Example" << std::endl;
    
    // Create a container VFS for our kernel
    std::string container_path = "./kernel_container.hcon";
    
    // Generate a random encryption key (for demonstration purposes)
    hydra::vfs::EncryptionKey key = {};
    for (size_t i = 0; i < key.size(); i++) {
        key[i] = static_cast<uint8_t>(rand() % 256);
    }
    
    // Set resource limits
    hydra::vfs::ResourceLimits limits;
    limits.max_file_size = 10 * 1024 * 1024;     // 10MB
    limits.max_file_count = 1000;
    limits.max_storage_size = 100 * 1024 * 1024; // 100MB
    
    // Create the container
    auto container_vfs = hydra::vfs::create_container_vfs(
        container_path,
        key,
        nullptr,  // Use default crypto provider
        hydra::vfs::SecurityLevel::STANDARD,
        limits
    );
    
    if (!container_vfs) {
        std::cerr << "Failed to create container VFS" << std::endl;
        return 1;
    }
    
    // Create the kernel
    kernel::HydraKernel kernel(container_vfs);
    
    // Set isolation mode (0 = none, 1 = partial, 2 = complete)
    // For this example, we'll use partial isolation to allow port forwarding
    kernel.setIsolationMode(1);
    
    // Start the kernel
    kernel.start();
    
    // Create processes
    auto server_proc = kernel.createProcess("server");
    auto client_proc = kernel.createProcess("client");
    auto web_server_proc = kernel.createProcess("web_server");
    
    if (!server_proc || !client_proc || !web_server_proc) {
        std::cerr << "Failed to create processes" << std::endl;
        kernel.stop();
        return 1;
    }
    
    // Set environment variables
    server_proc->setEnvironmentVariable("PORT", "8080");
    web_server_proc->setEnvironmentVariable("PORT", "8081");
    
    // Create a set of commands for the client to send
    std::vector<std::string> commands = {
        "ping",
        "time",
        "count",
        "unknown",
        "exit"
    };
    
    // Start the processes
    server_proc->executeFunction([](){ return server_process(8080); });
    web_server_proc->executeFunction([](){ return web_server_process(8081); });
    
    // Wait for the server processes to initialize
    std::this_thread::sleep_for(std::chrono::seconds(1));
    
    // Start the client process
    client_proc->executeFunction([commands](){ return client_process(commands); });
    
    // Wait for the client to finish
    client_proc->wait();
    
    // Wait for the server processes to finish
    server_proc->wait();
    web_server_proc->wait();
    
    // Display process exit codes
    std::cout << "Client exit code: " << client_proc->getExitCode() << std::endl;
    std::cout << "Server exit code: " << server_proc->getExitCode() << std::endl;
    std::cout << "Web server exit code: " << web_server_proc->getExitCode() << std::endl;
    
    // Display kernel statistics
    auto stats = kernel.getStats();
    std::cout << "Kernel statistics:" << std::endl;
    std::cout << "  Total processes: " << stats.total_processes << std::endl;
    std::cout << "  Running processes: " << stats.running_processes << std::endl;
    std::cout << "  Total memory usage: " << stats.total_memory_usage << " bytes" << std::endl;
    std::cout << "  Total CPU usage: " << stats.total_cpu_usage * 100.0f << "%" << std::endl;
    std::cout << "  Total threads: " << stats.total_threads << std::endl;
    std::cout << "  Total ports: " << stats.total_ports << std::endl;
    std::cout << "  Uptime: " << stats.uptime / 1000 << " seconds" << std::endl;
    
    // Stop the kernel
    kernel.stop();
    
    std::cout << "Hydra Kernel Example completed" << std::endl;
    return 0;
}
