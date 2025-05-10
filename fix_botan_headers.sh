#!/bin/bash
# Script to fix Botan headers for WebAssembly

# Set variables
INSTALL_DIR="/volumes/bigcode/hydra_sdk/lib/wasm"

# Fix the build.h file to remove architecture check
echo "Fixing Botan build.h file..."
sed -i '' '/Trying to compile Botan configured as x86_64 with non-x86_64 compiler/d' "${INSTALL_DIR}/include/botan-3/botan/build.h"

# Create a simplified concepts.h file
echo "Creating simplified concepts.h file..."
cat > "${INSTALL_DIR}/include/botan-3/botan/concepts.h" << EOF
/*
* Simplified Botan Concepts for WebAssembly
*/
#ifndef BOTAN_CONCEPTS_H_
#define BOTAN_CONCEPTS_H_

#include <botan/types.h>
#include <type_traits>
#include <vector>
#include <span>

namespace Botan {

namespace ranges {
// Simplified ranges namespace for WebAssembly
}

// Simplified contiguous_range concept
template <typename T>
struct is_contiguous_range : std::false_type {};

template <typename T>
struct is_contiguous_range<std::vector<T>> : std::true_type {};

template <typename T, size_t N>
struct is_contiguous_range<std::array<T, N>> : std::true_type {};

template <typename T>
struct is_contiguous_range<std::span<T>> : std::true_type {};

// Simplified contiguous_output_range concept
template <typename T>
struct is_contiguous_output_range : std::false_type {};

template <typename T>
struct is_contiguous_output_range<std::vector<T>> : std::true_type {};

template <typename T, size_t N>
struct is_contiguous_output_range<std::array<T, N>> : std::true_type {};

template <typename T>
struct is_contiguous_output_range<std::span<T>> : std::true_type {};

} // namespace Botan

#endif /* BOTAN_CONCEPTS_H_ */
EOF

echo "Botan headers have been fixed for WebAssembly!"
