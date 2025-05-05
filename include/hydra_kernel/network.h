#ifndef HYDRA_KERNEL_NETWORK_H
#define HYDRA_KERNEL_NETWORK_H

#include <string>
#include <vector>
#include <thread>
#include <atomic>
#include <mutex>
#include <unordered_map>
#include <functional>
#include <memory>

namespace hydra {
namespace kernel {

/**
 * @brief Connection statistics
 */
struct ConnectionStats {
    uint64_t bytes_received = 0;
    uint64_t bytes_sent = 0;
    uint64_t packets_received = 0;
    uint64_t packets_sent = 0;
    uint64_t connect_time = 0;    // Connection establishment time
    uint64_t last_active_time = 0; // Last activity time
};

/**
 * @brief Network connection
 */
class Connection {
public:
    /**
     * @brief Constructor
     * 
     * @param connection_id Unique connection identifier
     * @param socket_fd Socket file descriptor
     */
    Connection(int connection_id, int socket_fd);
    
    /**
     * @brief Destructor
     */
    ~Connection();
    
    /**
     * @brief Get the connection ID
     */
    int getID() const;
    
    /**
     * @brief Send data over the connection
     * 
     * @param data Data to send
     * @param size Size of data
     * @return Number of bytes sent, -1 on error
     */
    ssize_t send(const uint8_t* data, size_t size);
    
    /**
     * @brief Receive data from the connection
     * 
     * @param buffer Buffer to receive data
     * @param size Size of buffer
     * @return Number of bytes received, -1 on error, 0 on connection closed
     */
    ssize_t receive(uint8_t* buffer, size_t size);
    
    /**
     * @brief Close the connection
     */
    void close();
    
    /**
     * @brief Check if connection is active
     */
    bool isActive() const;
    
    /**
     * @brief Get connection statistics
     */
    ConnectionStats getStats() const;
    
    /**
     * @brief Get the socket file descriptor
     */
    int getSocketFd() const { return m_socket_fd; }
    
private:
    int m_id;
    int m_socket_fd;
    std::atomic<bool> m_active;
    ConnectionStats m_stats;
    std::mutex m_mutex;
};

/**
 * @brief Port forwarding service
 */
class PortForwarder {
public:
    /**
     * @brief Constructor
     */
    PortForwarder();
    
    /**
     * @brief Destructor
     */
    ~PortForwarder();
    
    /**
     * @brief Add a port mapping
     * 
     * @param internal_port Internal port number
     * @param external_port External port number
     * @param protocol Protocol ("tcp" or "udp")
     * @return True if mapping was added successfully
     */
    bool addPortMapping(int internal_port, int external_port, const std::string& protocol = "tcp");
    
    /**
     * @brief Remove a port mapping
     * 
     * @param internal_port Internal port number
     * @return True if mapping was removed successfully
     */
    bool removePortMapping(int internal_port);
    
    /**
     * @brief Start all port forwarders
     * 
     * @return True if started successfully
     */
    bool start();
    
    /**
     * @brief Stop all port forwarders
     */
    void stop();
    
    /**
     * @brief Check if port forwarder is running
     */
    bool isRunning() const;
    
    /**
     * @brief Set the data handler for a specific port
     * 
     * @param internal_port Internal port number
     * @param handler Data handler function
     * @return True if handler was set successfully
     */
    bool setDataHandler(int internal_port, std::function<void(const uint8_t*, size_t)> handler);
    
    /**
     * @brief Send data to a connection
     * 
     * @param internal_port Internal port number
     * @param connection_id Connection ID
     * @param data Data to send
     * @param size Size of data
     * @return Number of bytes sent, -1 on error
     */
    ssize_t sendData(int internal_port, int connection_id, const uint8_t* data, size_t size);
    
    /**
     * @brief Get active connections for a port
     * 
     * @param internal_port Internal port number
     * @return Vector of connection IDs
     */
    std::vector<int> getConnections(int internal_port);
    
    /**
     * @brief Get connection statistics
     * 
     * @param internal_port Internal port number
     * @param connection_id Connection ID
     * @return Connection statistics
     */
    ConnectionStats getConnectionStats(int internal_port, int connection_id);

private:
    struct PortMapping {
        int internal_port;
        int external_port;
        std::string protocol;
        std::thread thread;
        std::atomic<bool> running;
        int socket_fd;
        std::unordered_map<int, std::shared_ptr<Connection>> connections;
        std::function<void(const uint8_t*, size_t)> data_handler;
        std::mutex mutex;
    };
    
    std::unordered_map<int, std::shared_ptr<PortMapping>> m_mappings;
    std::atomic<bool> m_running;
    std::mutex m_mutex;
    
    void forwardingThread(std::shared_ptr<PortMapping> mapping);
    int createServerSocket(int port, const std::string& protocol);
    int acceptConnection(int server_socket);
    static int next_connection_id;
    static std::mutex connection_id_mutex;
};

/**
 * @brief Virtual network interface
 */
class NetworkInterface {
public:
    /**
     * @brief Constructor
     * 
     * @param name Interface name
     * @param ip_address IP address
     * @param netmask Network mask
     * @param mtu Maximum transmission unit
     */
    NetworkInterface(const std::string& name, const std::string& ip_address, 
                    const std::string& netmask, int mtu = 1500);
    
    /**
     * @brief Destructor
     */
    ~NetworkInterface();
    
    /**
     * @brief Get the interface name
     */
    std::string getName() const;
    
    /**
     * @brief Get the IP address
     */
    std::string getIPAddress() const;
    
    /**
     * @brief Set the IP address
     */
    void setIPAddress(const std::string& ip_address);
    
    /**
     * @brief Get the network mask
     */
    std::string getNetmask() const;
    
    /**
     * @brief Set the network mask
     */
    void setNetmask(const std::string& netmask);
    
    /**
     * @brief Get the MTU
     */
    int getMTU() const;
    
    /**
     * @brief Set the MTU
     */
    void setMTU(int mtu);
    
    /**
     * @brief Check if interface is up
     */
    bool isUp() const;
    
    /**
     * @brief Set interface up or down
     */
    void setUp(bool up);
    
    /**
     * @brief Get interface statistics
     */
    ConnectionStats getStats() const;
    
    /**
     * @brief Send packet through the interface
     * 
     * @param data Packet data
     * @param size Size of data
     * @return Number of bytes sent, -1 on error
     */
    ssize_t sendPacket(const uint8_t* data, size_t size);
    
    /**
     * @brief Set packet handler
     * 
     * @param handler Function to call when a packet is received
     */
    void setPacketHandler(std::function<void(const uint8_t*, size_t)> handler);

private:
    std::string m_name;
    std::string m_ip_address;
    std::string m_netmask;
    int m_mtu;
    std::atomic<bool> m_up;
    ConnectionStats m_stats;
    std::function<void(const uint8_t*, size_t)> m_packet_handler;
    std::mutex m_mutex;
};

} // namespace kernel
} // namespace hydra

#endif // HYDRA_KERNEL_NETWORK_H
