// Mock Botan mem_ops.h header for WebAssembly build
#pragma once

#include <cstring>
#include <vector>
#include <algorithm>

namespace Botan {

template<typename T>
inline void copy_mem(T* out, const T* in, size_t n)
{
    if(in != nullptr && n > 0)
        std::memmove(out, in, sizeof(T)*n);
}

template<typename T>
inline void clear_mem(T* ptr, size_t n)
{
    if(ptr != nullptr && n > 0)
        std::memset(ptr, 0, sizeof(T)*n);
}

template<typename T>
inline void set_mem(T* ptr, size_t n, uint8_t val)
{
    if(ptr != nullptr && n > 0)
        std::memset(ptr, val, sizeof(T)*n);
}

template<typename T>
inline bool same_mem(const T* p1, const T* p2, size_t n)
{
    if(n == 0)
        return true;
    if(p1 == p2)
        return true;
    if(p1 == nullptr || p2 == nullptr)
        return false;
    return std::memcmp(p1, p2, sizeof(T)*n) == 0;
}

} // namespace Botan
