#include <hydra_kernel/network.h>
#include <iostream>
#include <cstring>
#include <thread>
#include <chrono>
#include <algorithm>

// Platform-specific includes
#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    typedef int socklen_t;
    #pragma comment(lib, "Ws2_32.lib")
#else
    #include <unistd.h>
    #include <sys/socket.h>
    #include <sys/types.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <fcntl.h>
    #include <sys/select.h>
#endif

namespace hydra {
namespace kernel {

// Static member initialization
int PortForwarder::next_connection_id = 1;
std::mutex PortForwarder::connection_id_mutex;

// Platform-specific socket functions
#ifdef _WIN32
    // Windows socket initialization
    bool initWinsock() {
        WSADATA wsaData;
        return WSAStartup(MAKEWORD(2, 2), &wsaData) == 0;
    }
    
    // Windows socket cleanup
    void cleanupWinsock() {
        WSACleanup();
    }
    
    // Windows socket close
    void closeSocket(int socket) {
        closesocket(socket);
    }
    
    // Windows set non-blocking
    bool setNonBlocking(int socket) {
        u_long mode = 1;
        return ioctlsocket(socket, FIONBIO, &mode) == 0;
    }
    
    // Windows get last error
    int getLastError() {
        return WSAGetLastError();
    }
#else
    // Unix socket initialization (no-op)
    bool initWinsock() {
        return true;
    }
    
    // Unix socket cleanup (no-op)
    void cleanupWinsock() {
    }
    
    // Unix socket close
    void closeSocket(int socket) {
        close(socket);
    }
    
    // Unix set non-blocking
    bool setNonBlocking(int socket) {
        int flags = fcntl(socket, F_GETFL, 0);
        return fcntl(socket, F_SETFL, flags | O_NONBLOCK) != -1;
    }
    
    // Unix get last error
    int getLastError() {
        return errno;
    }
#endif

// Connection implementation
Connection::Connection(int connection_id, int socket_fd)
    : m_id(connection_id),
      m_socket_fd(socket_fd),
      m_active(true) {
    
    // Initialize stats
    m_stats.connect_time = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    m_stats.last_active_time = m_stats.connect_time;
}

Connection::~Connection() {
    // Close the connection if still active
    if (m_active.load()) {
        close();
    }
}

int Connection::getID() const {
    return m_id;
}

ssize_t Connection::send(const uint8_t* data, size_t size) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (!m_active.load()) {
        return -1;
    }
    
    // Send data
    ssize_t bytes_sent = ::send(m_socket_fd, reinterpret_cast<const char*>(data), size, 0);
    
    if (bytes_sent > 0) {
        // Update stats
        m_stats.bytes_sent += bytes_sent;
        m_stats.packets_sent++;
        m_stats.last_active_time = std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
    }
    
    return bytes_sent;
}

ssize_t Connection::receive(uint8_t* buffer, size_t size) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (!m_active.load()) {
        return -1;
    }
    
    // Receive data
    ssize_t bytes_received = recv(m_socket_fd, reinterpret_cast<char*>(buffer), size, 0);
    
    if (bytes_received > 0) {
        // Update stats
        m_stats.bytes_received += bytes_received;
        m_stats.packets_received++;
        m_stats.last_active_time = std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
    } else if (bytes_received == 0) {
        // Connection closed
        m_active.store(false);
    }
    
    return bytes_received;
}

void Connection::close() {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (!m_active.load()) {
        return;
    }
    
    // Close the socket
    closeSocket(m_socket_fd);
    
    // Update state
    m_active.store(false);
}

bool Connection::isActive() const {
    return m_active.load();
}

ConnectionStats Connection::getStats() const {
    return m_stats;
}

// PortForwarder implementation
PortForwarder::PortForwarder() 
    : m_running(false) {
    
    // Initialize socket library
    initWinsock();
}

PortForwarder::~PortForwarder() {
    // Stop forwarding if running
    if (m_running.load()) {
        stop();
    }
    
    // Cleanup socket library
    cleanupWinsock();
}

bool PortForwarder::addPortMapping(int internal_port, int external_port, const std::string& protocol) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    // Check if port mapping already exists
    if (m_mappings.find(internal_port) != m_mappings.end()) {
        return false;
    }
    
    // Create new port mapping
    auto mapping = std::make_shared<PortMapping>();
    mapping->internal_port = internal_port;
    mapping->external_port = external_port;
    mapping->protocol = protocol;
    mapping->running = false;
    mapping->socket_fd = -1;
    
    // Store the mapping
    m_mappings[internal_port] = mapping;
    
    return true;
}

bool PortForwarder::removePortMapping(int internal_port) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    // Check if port mapping exists
    auto it = m_mappings.find(internal_port);
    if (it == m_mappings.end()) {
        return false;
    }
    
    // Stop the thread if running
    auto mapping = it->second;
    if (mapping->running.load()) {
        mapping->running.store(false);
        
        if (mapping->thread.joinable()) {
            mapping->thread.join();
        }
        
        // Close the socket
        if (mapping->socket_fd != -1) {
            closeSocket(mapping->socket_fd);
            mapping->socket_fd = -1;
        }
    }
    
    // Remove the mapping
    m_mappings.erase(it);
    
    return true;
}

bool PortForwarder::start() {
    // Check if already running
    if (m_running.load()) {
        return true;
    }
    
    std::cout << "Starting port forwarder..." << std::endl;
    
    std::lock_guard<std::mutex> lock(m_mutex);
    
    m_running.store(true);
    
    // Start all port mappings
    for (auto& pair : m_mappings) {
        auto mapping = pair.second;
        
        // Skip if already running
        if (mapping->running.load()) {
            continue;
        }
        
        // Create server socket
        mapping->socket_fd = createServerSocket(mapping->external_port, mapping->protocol);
        if (mapping->socket_fd == -1) {
            std::cerr << "Failed to create server socket for port " << mapping->external_port << std::endl;
            continue;
        }
        
        // Start forwarding thread
        mapping->running.store(true);
        mapping->thread = std::thread(&PortForwarder::forwardingThread, this, mapping);
    }
    
    return true;
}

void PortForwarder::stop() {
    // Check if already stopped
    if (!m_running.load()) {
        return;
    }
    
    std::cout << "Stopping port forwarder..." << std::endl;
    
    std::lock_guard<std::mutex> lock(m_mutex);
    
    m_running.store(false);
    
    // Stop all port mappings
    for (auto& pair : m_mappings) {
        auto mapping = pair.second;
        
        // Skip if already stopped
        if (!mapping->running.load()) {
            continue;
        }
        
        // Signal thread to stop
        mapping->running.store(false);
        
        // Close the socket to unblock any blocking calls
        if (mapping->socket_fd != -1) {
            closeSocket(mapping->socket_fd);
            mapping->socket_fd = -1;
        }
        
        // Wait for thread to complete
        if (mapping->thread.joinable()) {
            mapping->thread.join();
        }
    }
}

bool PortForwarder::isRunning() const {
    return m_running.load();
}

bool PortForwarder::setDataHandler(int internal_port, std::function<void(const uint8_t*, size_t)> handler) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    // Check if port mapping exists
    auto it = m_mappings.find(internal_port);
    if (it == m_mappings.end()) {
        return false;
    }
    
    // Set the data handler
    it->second->data_handler = handler;
    
    return true;
}

ssize_t PortForwarder::sendData(int internal_port, int connection_id, const uint8_t* data, size_t size) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    // Check if port mapping exists
    auto it = m_mappings.find(internal_port);
    if (it == m_mappings.end()) {
        return -1;
    }
    
    auto mapping = it->second;
    
    // Check if connection exists
    auto conn_it = mapping->connections.find(connection_id);
    if (conn_it == mapping->connections.end()) {
        return -1;
    }
    
    // Send data
    return conn_it->second->send(data, size);
}

std::vector<int> PortForwarder::getConnections(int internal_port) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    std::vector<int> connections;
    
    // Check if port mapping exists
    auto it = m_mappings.find(internal_port);
    if (it == m_mappings.end()) {
        return connections;
    }
    
    auto mapping = it->second;
    
    // Get all active connections
    for (const auto& pair : mapping->connections) {
        if (pair.second->isActive()) {
            connections.push_back(pair.first);
        }
    }
    
    return connections;
}

ConnectionStats PortForwarder::getConnectionStats(int internal_port, int connection_id) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    ConnectionStats stats;
    
    // Check if port mapping exists
    auto it = m_mappings.find(internal_port);
    if (it == m_mappings.end()) {
        return stats;
    }
    
    auto mapping = it->second;
    
    // Check if connection exists
    auto conn_it = mapping->connections.find(connection_id);
    if (conn_it == mapping->connections.end()) {
        return stats;
    }
    
    // Get connection stats
    return conn_it->second->getStats();
}

void PortForwarder::forwardingThread(std::shared_ptr<PortMapping> mapping) {
    std::cout << "Forwarding thread started for port " << mapping->external_port << std::endl;
    
    // Set up file descriptors for select
    fd_set read_fds, write_fds, except_fds;
    struct timeval timeout;
    
    while (mapping->running.load() && m_running.load()) {
        // Initialize file descriptor sets
        FD_ZERO(&read_fds);
        FD_ZERO(&write_fds);
        FD_ZERO(&except_fds);
        
        // Add server socket to read set
        FD_SET(mapping->socket_fd, &read_fds);
        
        // Add client sockets to read set
        int max_fd = mapping->socket_fd;
        {
            std::lock_guard<std::mutex> lock(mapping->mutex);
            
            for (auto it = mapping->connections.begin(); it != mapping->connections.end(); ) {
                if (!it->second->isActive()) {
                    // Remove inactive connections
                    it = mapping->connections.erase(it);
                    continue;
                }
                
                int socket_fd = it->second->getSocketFd();
                FD_SET(socket_fd, &read_fds);
                FD_SET(socket_fd, &except_fds);
                max_fd = std::max(max_fd, socket_fd);
                
                ++it;
            }
        }
        
        // Set timeout
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;
        
        // Wait for activity
        int activity = select(max_fd + 1, &read_fds, &write_fds, &except_fds, &timeout);
        
        if (activity < 0) {
            // Error
            break;
        }
        
        if (activity == 0) {
            // Timeout, continue
            continue;
        }
        
        // Check for incoming connections
        if (FD_ISSET(mapping->socket_fd, &read_fds)) {
            // Accept new connection
            int client_socket = acceptConnection(mapping->socket_fd);
            if (client_socket >= 0) {
                // Generate connection ID
                int connection_id;
                {
                    std::lock_guard<std::mutex> lock(connection_id_mutex);
                    connection_id = next_connection_id++;
                }
                
                // Create connection
                auto connection = std::make_shared<Connection>(connection_id, client_socket);
                
                // Add to connections
                std::lock_guard<std::mutex> lock(mapping->mutex);
                mapping->connections[connection_id] = connection;
                
                std::cout << "New connection " << connection_id << " on port " << mapping->external_port << std::endl;
            }
        }
        
        // Check for data on client sockets
        {
            std::lock_guard<std::mutex> lock(mapping->mutex);
            
            for (auto& pair : mapping->connections) {
                int socket_fd = pair.second->getSocketFd();
                
                if (FD_ISSET(socket_fd, &read_fds)) {
                    // Data available
                    uint8_t buffer[4096];
                    ssize_t bytes_read = pair.second->receive(buffer, sizeof(buffer));
                    
                    if (bytes_read > 0) {
                        // Forward data to handler if set
                        if (mapping->data_handler) {
                            mapping->data_handler(buffer, bytes_read);
                        }
                    } else if (bytes_read <= 0) {
                        // Connection closed or error
                        pair.second->close();
                    }
                }
                
                if (FD_ISSET(socket_fd, &except_fds)) {
                    // Exception on socket
                    pair.second->close();
                }
            }
        }
    }
    
    // Close all connections
    {
        std::lock_guard<std::mutex> lock(mapping->mutex);
        
        for (auto& pair : mapping->connections) {
            pair.second->close();
        }
        
        mapping->connections.clear();
    }
    
    // Close server socket
    if (mapping->socket_fd != -1) {
        closeSocket(mapping->socket_fd);
        mapping->socket_fd = -1;
    }
    
    mapping->running.store(false);
    
    std::cout << "Forwarding thread stopped for port " << mapping->external_port << std::endl;
}

int PortForwarder::createServerSocket(int port, const std::string& protocol) {
    // Create socket
    int socket_fd;
    if (protocol == "tcp") {
        socket_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    } else if (protocol == "udp") {
        socket_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    } else {
        std::cerr << "Unsupported protocol: " << protocol << std::endl;
        return -1;
    }
    
    if (socket_fd < 0) {
        std::cerr << "Failed to create socket: " << getLastError() << std::endl;
        return -1;
    }
    
    // Set socket options
    int option = 1;
    if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<char*>(&option), sizeof(option)) < 0) {
        std::cerr << "Failed to set socket options: " << getLastError() << std::endl;
        closeSocket(socket_fd);
        return -1;
    }
    
    // Set non-blocking mode
    if (!setNonBlocking(socket_fd)) {
        std::cerr << "Failed to set non-blocking mode: " << getLastError() << std::endl;
        closeSocket(socket_fd);
        return -1;
    }
    
    // Bind socket
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);
    
    if (bind(socket_fd, reinterpret_cast<struct sockaddr*>(&server_addr), sizeof(server_addr)) < 0) {
        std::cerr << "Failed to bind socket: " << getLastError() << std::endl;
        closeSocket(socket_fd);
        return -1;
    }
    
    // Listen for connections (TCP only)
    if (protocol == "tcp") {
        if (listen(socket_fd, 10) < 0) {
            std::cerr << "Failed to listen on socket: " << getLastError() << std::endl;
            closeSocket(socket_fd);
            return -1;
        }
    }
    
    std::cout << "Server socket created on port " << port << " (" << protocol << ")" << std::endl;
    
    return socket_fd;
}

int PortForwarder::acceptConnection(int server_socket) {
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
    
    int client_socket = accept(server_socket, reinterpret_cast<struct sockaddr*>(&client_addr), &addr_len);
    
    if (client_socket < 0) {
        if (getLastError() == EWOULDBLOCK || getLastError() == EAGAIN) {
            // No pending connections
            return -1;
        }
        
        std::cerr << "Failed to accept connection: " << getLastError() << std::endl;
        return -1;
    }
    
    // Set non-blocking mode
    if (!setNonBlocking(client_socket)) {
        std::cerr << "Failed to set non-blocking mode for client socket: " << getLastError() << std::endl;
        closeSocket(client_socket);
        return -1;
    }
    
    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, sizeof(client_ip));
    
    std::cout << "Accepted connection from " << client_ip << ":" << ntohs(client_addr.sin_port) << std::endl;
    
    return client_socket;
}

// NetworkInterface implementation
NetworkInterface::NetworkInterface(const std::string& name, const std::string& ip_address, 
                                 const std::string& netmask, int mtu)
    : m_name(name),
      m_ip_address(ip_address),
      m_netmask(netmask),
      m_mtu(mtu),
      m_up(false) {
    
    std::cout << "Network interface created: " << name << " (" << ip_address << "/" << netmask << ")" << std::endl;
}

NetworkInterface::~NetworkInterface() {
    std::cout << "Network interface destroyed: " << m_name << std::endl;
}

std::string NetworkInterface::getName() const {
    return m_name;
}

std::string NetworkInterface::getIPAddress() const {
    return m_ip_address;
}

void NetworkInterface::setIPAddress(const std::string& ip_address) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_ip_address = ip_address;
}

std::string NetworkInterface::getNetmask() const {
    return m_netmask;
}

void NetworkInterface::setNetmask(const std::string& netmask) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_netmask = netmask;
}

int NetworkInterface::getMTU() const {
    return m_mtu;
}

void NetworkInterface::setMTU(int mtu) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_mtu = mtu;
}

bool NetworkInterface::isUp() const {
    return m_up.load();
}

void NetworkInterface::setUp(bool up) {
    m_up.store(up);
}

ConnectionStats NetworkInterface::getStats() const {
    return m_stats;
}

ssize_t NetworkInterface::sendPacket(const uint8_t* data, size_t size) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (!m_up.load()) {
        return -1;
    }
    
    // In a real implementation, we would send the packet over the network
    // For demonstration purposes, we'll just update the stats
    
    m_stats.bytes_sent += size;
    m_stats.packets_sent++;
    m_stats.last_active_time = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    
    // If a packet handler is set, call it to simulate loopback
    if (m_packet_handler) {
        m_packet_handler(data, size);
    }
    
    return size;
}

void NetworkInterface::setPacketHandler(std::function<void(const uint8_t*, size_t)> handler) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_packet_handler = handler;
}

} // namespace kernel
} // namespace hydra
