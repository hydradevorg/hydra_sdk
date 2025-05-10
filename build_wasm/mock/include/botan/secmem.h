// Mock Botan secmem.h header for WebAssembly build
#pragma once

#include <vector>
#include <memory>
#include <cstdint>

namespace Botan {

// Simplified secure_allocator without C++20 requires
template<typename T>
class secure_allocator
{
public:
    using value_type = T;
    using pointer = T*;
    using const_pointer = const T*;
    using reference = T&;
    using const_reference = const T&;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;

    secure_allocator() noexcept = default;
    secure_allocator(const secure_allocator&) noexcept = default;
    template<typename U>
    secure_allocator(const secure_allocator<U>&) noexcept {}

    ~secure_allocator() noexcept = default;

    pointer allocate(size_type n)
    {
        return static_cast<pointer>(::operator new(n * sizeof(T)));
    }

    void deallocate(pointer p, size_type n)
    {
        ::operator delete(p);
    }
};

template<typename T, typename U>
bool operator==(const secure_allocator<T>&, const secure_allocator<U>&) noexcept
{
    return true;
}

template<typename T, typename U>
bool operator!=(const secure_allocator<T>&, const secure_allocator<U>&) noexcept
{
    return false;
}

// Define secure containers
template<typename T>
using secure_vector = std::vector<T, secure_allocator<T>>;

template<typename T>
using secure_deque = std::deque<T, secure_allocator<T>>;

} // namespace Botan
