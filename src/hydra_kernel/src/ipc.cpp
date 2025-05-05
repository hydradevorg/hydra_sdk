#include <hydra_kernel/ipc.h>
#include <iostream>
#include <chrono>
#include <algorithm>

namespace hydra {
namespace kernel {

// Static member initialization for IPCManager
std::unordered_map<std::string, std::shared_ptr<MessageQueue>> IPCManager::s_message_queues;
std::unordered_map<std::string, std::shared_ptr<SharedMemory>> IPCManager::s_shared_memory;
std::unordered_map<std::string, std::shared_ptr<Semaphore>> IPCManager::s_semaphores;
std::mutex IPCManager::s_mutex;

// MessageQueue implementation
MessageQueue::MessageQueue(const std::string& name)
    : m_name(name) {
    
    std::cout << "Message queue created: " << name << std::endl;
}

MessageQueue::~MessageQueue() {
    std::cout << "Message queue destroyed: " << m_name << std::endl;
}

bool MessageQueue::send(const std::vector<uint8_t>& message) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    // Add message to queue
    m_queue.push(message);
    
    // Notify any waiting threads
    m_condition.notify_one();
    
    return true;
}

std::optional<std::vector<uint8_t>> MessageQueue::receive(bool blocking, uint64_t timeout_ms) {
    std::unique_lock<std::mutex> lock(m_mutex);
    
    if (m_queue.empty()) {
        if (!blocking) {
            return std::nullopt;
        }
        
        if (timeout_ms == 0) {
            // Wait indefinitely
            m_condition.wait(lock, [this] {
                return !m_queue.empty();
            });
        } else {
            // Wait with timeout
            auto result = m_condition.wait_for(lock, std::chrono::milliseconds(timeout_ms), [this] {
                return !m_queue.empty();
            });
            
            if (!result) {
                // Timeout
                return std::nullopt;
            }
        }
    }
    
    // Get message from queue
    auto message = m_queue.front();
    m_queue.pop();
    
    return message;
}

size_t MessageQueue::getMessageCount() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_queue.size();
}

std::string MessageQueue::getName() const {
    return m_name;
}

void MessageQueue::clear() {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    // Clear the queue
    while (!m_queue.empty()) {
        m_queue.pop();
    }
}

bool MessageQueue::hasMessages() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return !m_queue.empty();
}

// SharedMemory implementation
SharedMemory::SharedMemory(const std::string& name, size_t size)
    : m_name(name),
      m_size(size),
      m_locked(false) {
    
    // Allocate memory
    m_data.resize(size);
    
    std::cout << "Shared memory created: " << name << " (" << size << " bytes)" << std::endl;
}

SharedMemory::~SharedMemory() {
    std::cout << "Shared memory destroyed: " << m_name << std::endl;
}

uint8_t* SharedMemory::getData() {
    return m_data.data();
}

const uint8_t* SharedMemory::getData() const {
    return m_data.data();
}

size_t SharedMemory::getSize() const {
    return m_size;
}

std::string SharedMemory::getName() const {
    return m_name;
}

bool SharedMemory::lock(uint64_t timeout_ms) {
    std::unique_lock<std::mutex> lock(m_mutex);
    
    if (m_locked) {
        if (timeout_ms == 0) {
            // Wait indefinitely
            m_condition.wait(lock, [this] {
                return !m_locked;
            });
        } else {
            // Wait with timeout
            auto result = m_condition.wait_for(lock, std::chrono::milliseconds(timeout_ms), [this] {
                return !m_locked;
            });
            
            if (!result) {
                // Timeout
                return false;
            }
        }
    }
    
    m_locked = true;
    return true;
}

bool SharedMemory::tryLock() {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (m_locked) {
        return false;
    }
    
    m_locked = true;
    return true;
}

void SharedMemory::unlock() {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    m_locked = false;
    
    // Notify any waiting threads
    m_condition.notify_one();
}

// Semaphore implementation
Semaphore::Semaphore(const std::string& name, int initial_value)
    : m_name(name),
      m_value(initial_value) {
    
    std::cout << "Semaphore created: " << name << " (initial value: " << initial_value << ")" << std::endl;
}

Semaphore::~Semaphore() {
    std::cout << "Semaphore destroyed: " << m_name << std::endl;
}

bool Semaphore::wait(uint64_t timeout_ms) {
    std::unique_lock<std::mutex> lock(m_mutex);
    
    if (m_value <= 0) {
        if (timeout_ms == 0) {
            // Wait indefinitely
            m_condition.wait(lock, [this] {
                return m_value > 0;
            });
        } else {
            // Wait with timeout
            auto result = m_condition.wait_for(lock, std::chrono::milliseconds(timeout_ms), [this] {
                return m_value > 0;
            });
            
            if (!result) {
                // Timeout
                return false;
            }
        }
    }
    
    m_value--;
    return true;
}

bool Semaphore::tryWait() {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (m_value <= 0) {
        return false;
    }
    
    m_value--;
    return true;
}

void Semaphore::signal() {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    m_value++;
    
    // Notify one waiting thread
    m_condition.notify_one();
}

std::string Semaphore::getName() const {
    return m_name;
}

int Semaphore::getValue() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_value;
}

// IPCManager implementation
std::shared_ptr<MessageQueue> IPCManager::getMessageQueue(const std::string& name) {
    std::lock_guard<std::mutex> lock(s_mutex);
    
    // Check if queue already exists
    auto it = s_message_queues.find(name);
    if (it != s_message_queues.end()) {
        return it->second;
    }
    
    // Create new queue
    auto queue = std::make_shared<MessageQueue>(name);
    s_message_queues[name] = queue;
    
    return queue;
}

std::shared_ptr<SharedMemory> IPCManager::getSharedMemory(const std::string& name, size_t size) {
    std::lock_guard<std::mutex> lock(s_mutex);
    
    // Check if shared memory already exists
    auto it = s_shared_memory.find(name);
    if (it != s_shared_memory.end()) {
        // Check if size matches
        if (it->second->getSize() != size) {
            std::cerr << "Shared memory size mismatch: " << name << " (requested: " << size 
                      << ", actual: " << it->second->getSize() << ")" << std::endl;
        }
        
        return it->second;
    }
    
    // Create new shared memory
    auto shared_memory = std::make_shared<SharedMemory>(name, size);
    s_shared_memory[name] = shared_memory;
    
    return shared_memory;
}

std::shared_ptr<Semaphore> IPCManager::getSemaphore(const std::string& name, int initial_value) {
    std::lock_guard<std::mutex> lock(s_mutex);
    
    // Check if semaphore already exists
    auto it = s_semaphores.find(name);
    if (it != s_semaphores.end()) {
        return it->second;
    }
    
    // Create new semaphore
    auto semaphore = std::make_shared<Semaphore>(name, initial_value);
    s_semaphores[name] = semaphore;
    
    return semaphore;
}

bool IPCManager::deleteMessageQueue(const std::string& name) {
    std::lock_guard<std::mutex> lock(s_mutex);
    
    // Check if queue exists
    auto it = s_message_queues.find(name);
    if (it == s_message_queues.end()) {
        return false;
    }
    
    // Remove queue
    s_message_queues.erase(it);
    
    return true;
}

bool IPCManager::deleteSharedMemory(const std::string& name) {
    std::lock_guard<std::mutex> lock(s_mutex);
    
    // Check if shared memory exists
    auto it = s_shared_memory.find(name);
    if (it == s_shared_memory.end()) {
        return false;
    }
    
    // Remove shared memory
    s_shared_memory.erase(it);
    
    return true;
}

bool IPCManager::deleteSemaphore(const std::string& name) {
    std::lock_guard<std::mutex> lock(s_mutex);
    
    // Check if semaphore exists
    auto it = s_semaphores.find(name);
    if (it == s_semaphores.end()) {
        return false;
    }
    
    // Remove semaphore
    s_semaphores.erase(it);
    
    return true;
}

std::vector<std::string> IPCManager::listMessageQueues() {
    std::lock_guard<std::mutex> lock(s_mutex);
    
    std::vector<std::string> names;
    for (const auto& pair : s_message_queues) {
        names.push_back(pair.first);
    }
    
    return names;
}

std::vector<std::string> IPCManager::listSharedMemory() {
    std::lock_guard<std::mutex> lock(s_mutex);
    
    std::vector<std::string> names;
    for (const auto& pair : s_shared_memory) {
        names.push_back(pair.first);
    }
    
    return names;
}

std::vector<std::string> IPCManager::listSemaphores() {
    std::lock_guard<std::mutex> lock(s_mutex);
    
    std::vector<std::string> names;
    for (const auto& pair : s_semaphores) {
        names.push_back(pair.first);
    }
    
    return names;
}

void IPCManager::clearAll() {
    std::lock_guard<std::mutex> lock(s_mutex);
    
    s_message_queues.clear();
    s_shared_memory.clear();
    s_semaphores.clear();
    
    std::cout << "All IPC objects cleared" << std::endl;
}

} // namespace kernel
} // namespace hydra
