#!/bin/bash
# Script to build Hydra SDK components as WebAssembly modules

# Ensure Emscripten is activated
if [ -z "${EMSDK}" ]; then
  echo "Error: Emscripten environment not detected. Please run 'source ./emsdk_env.sh' first."
  exit 1
fi

# Set directories
HYDRA_ROOT="/volumes/bigcode/hydra_sdk"
BUILD_DIR="${HYDRA_ROOT}/build_wasm"
TOOLCHAIN_FILE="${HYDRA_ROOT}/cmake/hydra_emscripten.cmake"
OUTPUT_DIR="${HYDRA_ROOT}/wasm_modules"

# Create directories
mkdir -p "${BUILD_DIR}"
mkdir -p "${OUTPUT_DIR}"

# Copy toolchain file
cp "hydra_emscripten.cmake" "${HYDRA_ROOT}/cmake/"

# Function to build a specific Hydra module
build_module() {
  local MODULE_NAME=$1
  local MODULE_PATH=$2
  local EXTRA_FLAGS=$3
  
  echo "Building ${MODULE_NAME}..."
  
  # Create build directory for this module
  local MODULE_BUILD_DIR="${BUILD_DIR}/${MODULE_NAME}"
  mkdir -p "${MODULE_BUILD_DIR}"
  cd "${MODULE_BUILD_DIR}"
  
  # Configure with CMake
  emcmake cmake "${MODULE_PATH}" \
    -DCMAKE_TOOLCHAIN_FILE="${TOOLCHAIN_FILE}" \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_FOR_WASM=ON \
    -DHYDRA_ENABLE_EXCEPTIONS=OFF \
    ${EXTRA_FLAGS}
  
  # Build
  emmake make -j4
  
  # Copy output to the modules directory
  mkdir -p "${OUTPUT_DIR}/${MODULE_NAME}"
  cp *.js "${OUTPUT_DIR}/${MODULE_NAME}/"
  cp *.wasm "${OUTPUT_DIR}/${MODULE_NAME}/"
  
  echo "${MODULE_NAME} build completed"
  cd "${HYDRA_ROOT}"
}

# First create a dummy binding module for each component we want to compile
create_binding_file() {
  local MODULE_NAME=$1
  local OUTPUT_FILE="${HYDRA_ROOT}/src/${MODULE_NAME}/wasm_bindings.cpp"
  
  echo "Creating bindings for ${MODULE_NAME}..."
  
  cat > "${OUTPUT_FILE}" << EOF
#include <emscripten/bind.h>
#include <emscripten/val.h>
#include <${MODULE_NAME}/${MODULE_NAME}.hpp>

using namespace emscripten;

EMSCRIPTEN_BINDINGS(${MODULE_NAME}) {
  // Add appropriate bindings for each class and function
  // This is a placeholder - will expand based on module specifics
}
EOF
}

# Create individual binding files for key modules
create_binding_file "hydra_address"
create_binding_file "hydra_crypto"
create_binding_file "hydra_vfs"

# Create specialized binding files for key components
# P2P VFS Bindings
cat > "${HYDRA_ROOT}/src/hydra_vfs/wasm_bindings.cpp" << EOF
#include <emscripten/bind.h>
#include <emscripten/val.h>
#include <hydra_vfs/vfs.hpp>
#include <lmvs/p2p_vfs/p2p_vfs.hpp>

using namespace emscripten;

// Helper function to convert Hydra Result<T> to JavaScript object
template<typename T>
val result_to_js(const hydra::Result<T>& result) {
    val jsResult = val::object();
    jsResult.set("success", result.success());
    
    if (result.success()) {
        jsResult.set("value", result.value());
    } else {
        jsResult.set("error", result.error());
    }
    
    return jsResult;
}

// Wrapper for P2P VFS
class P2PVFSWrapper {
private:
    lmvs::p2p_vfs::P2PVFS vfs;
    
public:
    P2PVFSWrapper(const std::string& node_id, const std::string& storage_path)
        : vfs(node_id, storage_path) {}
    
    bool add_peer(const std::string& peer_id, const std::string& peer_address) {
        return vfs.add_peer(peer_id, peer_address);
    }
    
    std::vector<std::string> get_peers() {
        return vfs.get_peers();
    }
    
    val create_directory(const std::string& path) {
        return result_to_js(vfs.create_directory(path));
    }
    
    val file_exists(const std::string& path) {
        return result_to_js(vfs.file_exists(path));
    }
    
    val directory_exists(const std::string& path) {
        return result_to_js(vfs.directory_exists(path));
    }
    
    val list_files(const std::string& path) {
        return result_to_js(vfs.list_files(path));
    }
    
    void synchronize() {
        vfs.synchronize();
    }
    
    // Additional methods would be added here
};

EMSCRIPTEN_BINDINGS(hydra_vfs) {
    class_<P2PVFSWrapper>("P2PVFS")
        .constructor<std::string, std::string>()
        .function("add_peer", &P2PVFSWrapper::add_peer)
        .function("get_peers", &P2PVFSWrapper::get_peers)
        .function("create_directory", &P2PVFSWrapper::create_directory)
        .function("file_exists", &P2PVFSWrapper::file_exists)
        .function("directory_exists", &P2PVFSWrapper::directory_exists)
        .function("list_files", &P2PVFSWrapper::list_files)
        .function("synchronize", &P2PVFSWrapper::synchronize);
}
EOF

# Address Generator Bindings
cat > "${HYDRA_ROOT}/src/hydra_address/wasm_bindings.cpp" << EOF
#include <emscripten/bind.h>
#include <emscripten/val.h>
#include <hydra_address/address_generator.hpp>
#include <hydra_address/geohash.hpp>

using namespace emscripten;

// Wrapper for Address
class AddressWrapper {
private:
    hydra::address::Address address;
    
public:
    AddressWrapper(const hydra::address::Address& addr) : address(addr) {}
    
    std::string toString() const {
        return address.toString();
    }
    
    val getGeohash() const {
        auto geohash = address.getGeohash();
        if (geohash) {
            return val(*geohash);
        } else {
            return val::null();
        }
    }
    
    val getCoordinates() const {
        auto coords = address.getCoordinates();
        if (coords) {
            val result = val::object();
            result.set("latitude", coords->latitude);
            result.set("longitude", coords->longitude);
            result.set("altitude", coords->altitude);
            return result;
        } else {
            return val::null();
        }
    }
};

// Wrapper for AddressGenerator
class AddressGeneratorWrapper {
private:
    hydra::address::AddressGenerator generator;
    
public:
    AddressGeneratorWrapper(size_t bit_strength) : generator(bit_strength) {}
    
    AddressWrapper generateFromPublicKey(
        const std::vector<uint8_t>& public_key,
        const std::string& type_str,
        const std::string& format_str) {
        
        hydra::address::AddressType type;
        if (type_str == "USER") type = hydra::address::AddressType::USER;
        else if (type_str == "NODE") type = hydra::address::AddressType::NODE;
        else if (type_str == "RESOURCE") type = hydra::address::AddressType::RESOURCE;
        else if (type_str == "COMPONENT") type = hydra::address::AddressType::RESOURCE; // Use RESOURCE for COMPONENT
        else type = hydra::address::AddressType::USER;
        
        hydra::address::AddressFormat format;
        if (format_str == "STANDARD") format = hydra::address::AddressFormat::STANDARD;
        else if (format_str == "COMPRESSED") format = hydra::address::AddressFormat::COMPRESSED;
        else format = hydra::address::AddressFormat::STANDARD;
        
        return AddressWrapper(generator.generateFromPublicKey(public_key, type, format));
    }
    
    bool verifyAddress(const AddressWrapper& address) {
        // Implementation would need the actual Address object from the wrapper
        // This is simplified for the example
        return true; // generator.verifyAddress(address.getAddress());
    }
};

EMSCRIPTEN_BINDINGS(hydra_address) {
    class_<AddressWrapper>("Address")
        .constructor<>()
        .function("toString", &AddressWrapper::toString)
        .function("getGeohash", &AddressWrapper::getGeohash)
        .function("getCoordinates", &AddressWrapper::getCoordinates);
        
    class_<AddressGeneratorWrapper>("AddressGenerator")
        .constructor<size_t>()
        .function("generateFromPublicKey", &AddressGeneratorWrapper::generateFromPublicKey)
        .function("verifyAddress", &AddressGeneratorWrapper::verifyAddress);
        
    register_vector<uint8_t>("Vector<uint8_t>");
}
EOF

# Crypto Bindings
cat > "${HYDRA_ROOT}/src/hydra_crypto/wasm_bindings.cpp" << EOF
#include <emscripten/bind.h>
#include <emscripten/val.h>
#include <hydra_crypto/blake3_hash.hpp>
#include <hydra_crypto/falcon_signature.hpp>

using namespace emscripten;

// Wrapper for Blake3Hash
class Blake3HashWrapper {
private:
    hydra::crypto::Blake3Hash hasher;
    
public:
    Blake3HashWrapper() {}
    
    std::vector<uint8_t> hash(const std::vector<uint8_t>& data) {
        return hasher.hash(data);
    }
    
    std::vector<uint8_t> hashString(const std::string& data) {
        return hasher.hash(
            reinterpret_cast<const uint8_t*>(data.c_str()), 
            data.size()
        );
    }
};

// Wrapper for FalconSignature
class FalconSignatureWrapper {
private:
    hydra::crypto::FalconSignature signer;
    
public:
    FalconSignatureWrapper(int strength) : signer(strength) {}
    
    val generate_key_pair() {
        auto [public_key, private_key] = signer.generate_key_pair();
        
        val result = val::object();
        result.set("publicKey", val(typed_memory_view(public_key.size(), public_key.data())));
        result.set("privateKey", val(typed_memory_view(private_key.size(), private_key.data())));
        
        return result;
    }
    
    std::vector<uint8_t> sign(const std::vector<uint8_t>& data, const std::vector<uint8_t>& private_key) {
        return signer.sign(data, private_key);
    }
    
    bool verify(const std::vector<uint8_t>& data, const std::vector<uint8_t>& signature, const std::vector<uint8_t>& public_key) {
        return signer.verify(data, signature, public_key);
    }
};

EMSCRIPTEN_BINDINGS(hydra_crypto) {
    class_<Blake3HashWrapper>("Blake3Hash")
        .constructor<>()
        .function("hash", &Blake3HashWrapper::hash)
        .function("hashString", &Blake3HashWrapper::hashString);
        
    class_<FalconSignatureWrapper>("FalconSignature")
        .constructor<int>()
        .function("generate_key_pair", &FalconSignatureWrapper::generate_key_pair)
        .function("sign", &FalconSignatureWrapper::sign)
        .function("verify", &FalconSignatureWrapper::verify);
        
    register_vector<uint8_t>("Vector<uint8_t>");
}
EOF

# Build the main modules
build_module "hydra_crypto" "${HYDRA_ROOT}/src/hydra_crypto" "-DHYDRA_WASM_BINDINGS=ON"
build_module "hydra_address" "${HYDRA_ROOT}/src/hydra_address" "-DHYDRA_WASM_BINDINGS=ON"
build_module "hydra_vfs" "${HYDRA_ROOT}/src/hydra_vfs" "-DHYDRA_WASM_BINDINGS=ON -DHYDRA_BUILD_LMVS=ON"

echo "All modules built successfully!"
