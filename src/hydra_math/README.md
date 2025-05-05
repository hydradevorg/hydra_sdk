# Hydra Math Library

## Overview

The `hydra_math` library is a comprehensive mathematical toolkit that provides a wide range of mathematical operations and algorithms for the Hydra SDK. It includes implementations of various cryptographic primitives, number theory algorithms, lattice-based operations, and data structures for working with arbitrary-precision arithmetic.

## Key Components

### Arbitrary-Precision Arithmetic

- **BigInt**: A C++ wrapper around the GNU Multiple Precision (GMP) library that provides arbitrary-precision integer arithmetic with a convenient, object-oriented interface.
- **Rational**: A rational number implementation based on BigInt, allowing for exact representation of fractions.

### Modular Arithmetic

- **Modular**: Functions for modular arithmetic operations, including addition, subtraction, multiplication, and multiplicative inverse.

### Lattice Operations

- **LatticeUtils**: Utilities for lattice operations, including Gram-Schmidt orthogonalization and modular matrix operations.
- **LLL**: Implementation of the Lenstra-Lenstra-Lovász (LLL) lattice basis reduction algorithm.
- **BKZ**: Implementation of the Block Korkine-Zolotarev (BKZ) lattice basis reduction algorithm, which improves upon LLL.
- **LatticeSolver**: Solvers for various lattice problems, such as the Shortest Vector Problem (SVP) and Closest Vector Problem (CVP).

### Matrix Operations

- **ComplexMatrix**: A matrix implementation for complex numbers, extending Eigen's functionality.
- **MatrixUtils**: Utilities for matrix operations, including matrix exponentiation, matrix power, and various matrix norms.

### Cryptographic Primitives

- **Shamir**: Implementation of Shamir's Secret Sharing scheme, a cryptographic algorithm for securely sharing a secret among multiple parties.
- **Pedersen**: Implementation of Pedersen Commitments, a cryptographic primitive for committing to a value without revealing it.

### Data Compression

- **Huffman**: Implementation of Huffman coding, a lossless data compression algorithm.

### Galois Field Operations

- **GaloisVector**: Implementation of vectors over Galois fields, useful for error-correcting codes and cryptographic applications.

## Usage Examples

### BigInt Example

```cpp
#include "hydra_math/bigint.hpp"
#include <iostream>

using namespace hydra::math;

int main() {
    // Create BigInt from various sources
    BigInt a(123456789);
    BigInt b("987654321");
    
    // Arithmetic operations
    BigInt sum = a + b;
    BigInt product = a * b;
    
    // Output
    std::cout << "a + b = " << sum << std::endl;
    std::cout << "a * b = " << product << std::endl;
    
    return 0;
}
```

### Shamir's Secret Sharing Example

```cpp
#include "hydra_math/shamir.hpp"
#include <iostream>

using namespace hydra::math;

int main() {
    // Create a secret
    BigInt secret("12345678901234567890");
    
    // Parameters for the scheme
    int threshold = 3;  // Minimum shares needed to reconstruct
    int numShares = 5;  // Total number of shares to generate
    BigInt prime("21888242871839275222246405745257275088548364400416034343698204186575808495617");
    
    // Split the secret into shares
    auto shares = shamirSplit(secret, threshold, numShares, prime);
    
    // Print the shares
    for (size_t i = 0; i < shares.size(); ++i) {
        std::cout << "Share " << i+1 << ": (" << shares[i].x << ", " << shares[i].y << ")" << std::endl;
    }
    
    // Reconstruct the secret using a subset of shares
    std::vector<Share> subset = {shares[0], shares[2], shares[4]};
    BigInt reconstructed = shamirReconstruct(subset, prime);
    
    std::cout << "Original secret: " << secret << std::endl;
    std::cout << "Reconstructed secret: " << reconstructed << std::endl;
    
    return 0;
}
```

### LLL Lattice Reduction Example

```cpp
#include "hydra_math/lll.hpp"
#include <iostream>

using namespace hydra::math;
using namespace Eigen;

int main() {
    // Create a lattice basis
    MatrixXd basis(2, 2);
    basis << 201, 37,
             1648, 297;
    
    // Reduce the basis using LLL
    MatrixXd reduced = lllReduce(basis);
    
    std::cout << "Original basis:\n" << basis << std::endl;
    std::cout << "Reduced basis:\n" << reduced << std::endl;
    
    return 0;
}
```

## Dependencies

- **Eigen**: A C++ template library for linear algebra.
- **GMP**: The GNU Multiple Precision Arithmetic Library for arbitrary-precision arithmetic.

## Building

The `hydra_math` library is built as part of the Hydra SDK. See the main SDK documentation for build instructions.

## License

This library is part of the Hydra SDK and is subject to the same license terms as the SDK.
