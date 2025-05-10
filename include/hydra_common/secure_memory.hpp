#pragma once

#include <vector>
#include <memory>
#include <algorithm>
#include <cstring>
#include <type_traits>
#include <random>
#include "error_handling.hpp"

namespace hydra {
namespace common {

/**
 * @brief Securely wipes memory to prevent sensitive data leakage
 * 
 * This function overwrites the memory with random data and then zeros
 * to ensure sensitive data is not left in memory.
 * 
 * @param ptr Pointer to memory to wipe
 * @param size Size of memory to wipe in bytes
 */
inline void secureWipe(void* ptr, size_t size) {
    if (!ptr || size == 0) {
        return;
    }
    
    // First overwrite with random data
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<unsigned char> dist(0, 255);
    
    unsigned char* data = static_cast<unsigned char*>(ptr);
    for (size_t i = 0; i < size; ++i) {
        data[i] = dist(gen);
    }
    
    // Memory barrier to prevent compiler optimization
    std::atomic_thread_fence(std::memory_order_seq_cst);
    
    // Then overwrite with zeros
    std::memset(ptr, 0, size);
}

/**
 * @brief Securely wipes a container
 * 
 * @tparam Container Container type
 * @param container Container to wipe
 */
template<typename Container>
inline void secureWipeContainer(Container& container) {
    using value_type = typename Container::value_type;
    
    // Only allow trivially copyable types
    static_assert(std::is_trivially_copyable<value_type>::value, 
                 "secureWipeContainer only works with trivially copyable types");
    
    if (container.empty()) {
        return;
    }
    
    // Wipe the container's memory
    secureWipe(container.data(), container.size() * sizeof(value_type));
    
    // Clear the container
    container.clear();
}

/**
 * @brief Secure memory container for sensitive data
 * 
 * This class provides a container that automatically wipes its memory
 * when it goes out of scope or when clear() is called.
 * 
 * @tparam T Type of data to store
 */
template<typename T>
class SecureVector {
public:
    // Type definitions for STL compatibility
    using value_type = T;
    using size_type = size_t;
    using reference = T&;
    using const_reference = const T&;
    using pointer = T*;
    using const_pointer = const T*;
    using iterator = typename std::vector<T>::iterator;
    using const_iterator = typename std::vector<T>::const_iterator;
    
    /**
     * @brief Default constructor
     */
    SecureVector() = default;
    
    /**
     * @brief Constructor with size
     * @param size Initial size
     */
    explicit SecureVector(size_type size) : m_data(size) {}
    
    /**
     * @brief Constructor with size and initial value
     * @param size Initial size
     * @param value Initial value
     */
    SecureVector(size_type size, const T& value) : m_data(size, value) {}
    
    /**
     * @brief Constructor from initializer list
     * @param init Initializer list
     */
    SecureVector(std::initializer_list<T> init) : m_data(init) {}
    
    /**
     * @brief Constructor from iterators
     * @param first Iterator to first element
     * @param last Iterator past the last element
     */
    template<typename InputIt>
    SecureVector(InputIt first, InputIt last) : m_data(first, last) {}
    
    /**
     * @brief Copy constructor
     * @param other SecureVector to copy
     */
    SecureVector(const SecureVector& other) : m_data(other.m_data) {}
    
    /**
     * @brief Move constructor
     * @param other SecureVector to move
     */
    SecureVector(SecureVector&& other) noexcept : m_data(std::move(other.m_data)) {}
    
    /**
     * @brief Destructor
     * 
     * Automatically wipes the memory when the object is destroyed.
     */
    ~SecureVector() {
        clear();
    }
    
    /**
     * @brief Copy assignment operator
     * @param other SecureVector to copy
     * @return Reference to this
     */
    SecureVector& operator=(const SecureVector& other) {
        if (this != &other) {
            clear();
            m_data = other.m_data;
        }
        return *this;
    }
    
    /**
     * @brief Move assignment operator
     * @param other SecureVector to move
     * @return Reference to this
     */
    SecureVector& operator=(SecureVector&& other) noexcept {
        if (this != &other) {
            clear();
            m_data = std::move(other.m_data);
        }
        return *this;
    }
    
    /**
     * @brief Get the size of the container
     * @return Size
     */
    size_type size() const noexcept {
        return m_data.size();
    }
    
    /**
     * @brief Check if the container is empty
     * @return True if empty
     */
    bool empty() const noexcept {
        return m_data.empty();
    }
    
    /**
     * @brief Get the capacity of the container
     * @return Capacity
     */
    size_type capacity() const noexcept {
        return m_data.capacity();
    }
    
    /**
     * @brief Reserve memory
     * @param new_cap New capacity
     */
    void reserve(size_type new_cap) {
        m_data.reserve(new_cap);
    }
    
    /**
     * @brief Resize the container
     * @param count New size
     */
    void resize(size_type count) {
        m_data.resize(count);
    }
    
    /**
     * @brief Resize the container with a value
     * @param count New size
     * @param value Value to fill new elements
     */
    void resize(size_type count, const T& value) {
        m_data.resize(count, value);
    }
    
    /**
     * @brief Clear the container and wipe its memory
     */
    void clear() {
        if (!m_data.empty()) {
            secureWipeContainer(m_data);
        }
    }
    
    /**
     * @brief Access element at index
     * @param pos Index
     * @return Reference to element
     */
    reference operator[](size_type pos) {
        return m_data[pos];
    }
    
    /**
     * @brief Access element at index (const)
     * @param pos Index
     * @return Const reference to element
     */
    const_reference operator[](size_type pos) const {
        return m_data[pos];
    }
    
    /**
     * @brief Access element at index with bounds checking
     * @param pos Index
     * @return Reference to element
     */
    reference at(size_type pos) {
        return m_data.at(pos);
    }
    
    /**
     * @brief Access element at index with bounds checking (const)
     * @param pos Index
     * @return Const reference to element
     */
    const_reference at(size_type pos) const {
        return m_data.at(pos);
    }
    
    /**
     * @brief Get pointer to the underlying data
     * @return Pointer to data
     */
    pointer data() noexcept {
        return m_data.data();
    }
    
    /**
     * @brief Get pointer to the underlying data (const)
     * @return Const pointer to data
     */
    const_pointer data() const noexcept {
        return m_data.data();
    }
    
    /**
     * @brief Add element to the end
     * @param value Value to add
     */
    void push_back(const T& value) {
        m_data.push_back(value);
    }
    
    /**
     * @brief Add element to the end (move)
     * @param value Value to add
     */
    void push_back(T&& value) {
        m_data.push_back(std::move(value));
    }
    
    /**
     * @brief Remove last element
     */
    void pop_back() {
        m_data.pop_back();
    }
    
    /**
     * @brief Get iterator to the beginning
     * @return Iterator
     */
    iterator begin() noexcept {
        return m_data.begin();
    }
    
    /**
     * @brief Get const iterator to the beginning
     * @return Const iterator
     */
    const_iterator begin() const noexcept {
        return m_data.begin();
    }
    
    /**
     * @brief Get iterator to the end
     * @return Iterator
     */
    iterator end() noexcept {
        return m_data.end();
    }
    
    /**
     * @brief Get const iterator to the end
     * @return Const iterator
     */
    const_iterator end() const noexcept {
        return m_data.end();
    }
    
    /**
     * @brief Convert to std::vector (creates a copy)
     * @return Copy of the data as std::vector
     */
    std::vector<T> toVector() const {
        return m_data;
    }
    
private:
    std::vector<T> m_data;
};

/**
 * @brief Alias for secure byte vector
 */
using SecureByteVector = SecureVector<uint8_t>;

} // namespace common
} // namespace hydra
