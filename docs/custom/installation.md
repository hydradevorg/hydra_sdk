# Installation Guide

This guide provides instructions for building and installing the Hydra SDK from source.

## Prerequisites

Before building the Hydra SDK, ensure you have the following dependencies installed:

- **C++ Compiler**: GCC 9+ or Clang 10+
- **CMake**: Version 3.16 or higher
- **OpenSSL**: Version 1.1.1 or higher
- **GMP**: GNU Multiple Precision Arithmetic Library
- **yaml-cpp**: YAML parser and emitter for C++
- **Boost**: Version 1.70 or higher (for certain modules)

### Installing Dependencies on Ubuntu/Debian

```bash
sudo apt update
sudo apt install build-essential cmake libssl-dev libgmp-dev libyaml-cpp-dev libboost-all-dev
```

### Installing Dependencies on macOS

```bash
brew update
brew install cmake openssl gmp yaml-cpp boost
```

## Building from Source

1. Clone the repository:

```bash
git clone https://github.com/your-organization/hydra_sdk.git
cd hydra_sdk
```

2. Create a build directory:

```bash
mkdir -p build && cd build
```

3. Configure the build:

```bash
cmake ..
```

4. Build the SDK:

```bash
make
```

5. (Optional) Install the SDK:

```bash
sudo make install
```

## Verifying the Installation

After building the SDK, you can verify the installation by running the Hydra CLI:

```bash
./bin/hydra-cli --version
```

You should see output similar to:

```
Hydra CLI v1.0.0
Copyright (c) 2025 Hydra SDK Team
```

## Building with Specific Options

You can customize the build by passing options to CMake:

```bash
cmake -DBUILD_TESTS=ON -DBUILD_EXAMPLES=ON -DCMAKE_BUILD_TYPE=Release ..
```

Available options include:

- `BUILD_TESTS`: Build unit tests (ON/OFF)
- `BUILD_EXAMPLES`: Build example applications (ON/OFF)
- `CMAKE_BUILD_TYPE`: Build type (Debug/Release/RelWithDebInfo)
- `ENABLE_QUANTUM_RESISTANT`: Enable quantum-resistant algorithms (ON/OFF)
- `ENABLE_COMPRESSION`: Enable compression algorithms (ON/OFF)

## Troubleshooting

### Common Issues

#### Missing Dependencies

If you encounter errors about missing dependencies, ensure all required libraries are installed:

```bash
# On Ubuntu/Debian
sudo apt install libssl-dev libgmp-dev libyaml-cpp-dev libboost-all-dev

# On macOS
brew install openssl gmp yaml-cpp boost
```

#### Compilation Errors

If you encounter compilation errors, ensure you're using a compatible compiler:

```bash
# Check GCC version
gcc --version

# Check Clang version
clang --version
```

#### Linking Errors

If you encounter linking errors, ensure the libraries are in the correct path:

```bash
# On Ubuntu/Debian
export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH

# On macOS
export DYLD_LIBRARY_PATH=/usr/local/lib:$DYLD_LIBRARY_PATH
```

### Getting Help

If you continue to experience issues, please:

1. Check the [GitHub Issues](https://github.com/your-organization/hydra_sdk/issues) for similar problems
2. Join our [Discord community](https://discord.gg/hydra-sdk) for real-time support
3. Contact us at support@hydra-sdk.org
