# Layered Matrix and Vector System (LMVS)

This repository contains an implementation of the Layered Matrix and Vector System (LMVS) as described in the paper "Layered Matrix and Vector System for Secure, Scalable Distributed Computation" by Nicolas Cloutier.

## Overview

The Layered Matrix and Vector System is designed for managing secure, multi-dimensional data structures in distributed systems. It leverages layered matrices and vectors for secure consensus, fault tolerance, and data projection, enhancing the scalability and reliability of distributed computations.

## Features

- **Layered Vectors**: Multi-layer data representation
- **Layered Matrices**: Block matrix operations for layer interactions
- **Dimensional Projection**: Reduce dimensionality for efficient storage or processing
- **Secure Consensus**: Validate data integrity across distributed nodes
- **Fault Tolerance**: Reconstruct missing or corrupted layers
- **Layer Encryption**: Secure individual layers independently
- **Secret Sharing**: Split and reconstruct data securely

## Requirements

- C++17 compatible compiler
- CMake 3.10 or higher

## Building

```bash
mkdir build
cd build
cmake ..
make
```

## Running the Example

```bash
./examples/lmvs_example
```

## Running the Tests

```bash
./tests/lmvs_tests
```

## Usage

Here's a simple example of how to use the LMVS library:

```cpp
#include "lmvs/lmvs.hpp"
#include <iostream>
#include <vector>

int main() {
    // Initialize LMVS system
    const size_t num_layers = 3;
    const size_t dimension = 4;
    const size_t num_nodes = 5;
    const size_t threshold = 3;
    
    lmvs::LMVS system(num_layers, dimension, num_nodes, threshold);
    
    // Create a layered vector
    std::vector<std::vector<double>> data = {
        {1.0, 2.0, 3.0, 4.0},     // Layer 1
        {5.0, 6.0, 7.0, 8.0},     // Layer 2
        {9.0, 10.0, 11.0, 12.0}   // Layer 3
    };
    
    lmvs::LayeredVector vector = system.createVector(data);
    
    // Project the vector to a lower dimension
    const size_t output_dim = 2;
    lmvs::LayeredVector projected = system.projectVector(vector, output_dim);
    
    // Split the vector using secret sharing
    const size_t num_shares = 5;
    const size_t share_threshold = 3;
    auto shares = system.splitVector(vector, num_shares, share_threshold);
    
    // Reconstruct the vector from a subset of shares
    std::unordered_map<size_t, lmvs::LayeredVector> subset_shares;
    // ... add some shares to subset_shares ...
    
    lmvs::LayeredVector reconstructed = system.reconstructVector(subset_shares, share_threshold);
    
    return 0;
}
```

## License

This project is licensed under the MIT License - see the LICENSE file for details.

## Citation

If you use this code in your research, please cite the original paper:

```
Cloutier, N. (2023). Layered Matrix and Vector System for Secure, Scalable Distributed Computation.
```

## Acknowledgments

- The implementation is based on the mathematical foundations described in the paper by Nicolas Cloutier.
- Special thanks to the contributors and reviewers of the original paper.
