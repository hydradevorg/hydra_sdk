#!/bin/bash
# Script to create mock dependencies for the LMVS module

# Set variables
MOCK_DIR="/volumes/bigcode/hydra_sdk/build_wasm/mock"
LMVS_MOCK_DIR="${MOCK_DIR}/include/lmvs"
HYDRA_CRYPTO_MOCK_DIR="${MOCK_DIR}/include/hydra_crypto"

# Create directories
mkdir -p "${LMVS_MOCK_DIR}/security"
mkdir -p "${LMVS_MOCK_DIR}/p2p_vfs"
mkdir -p "${HYDRA_CRYPTO_MOCK_DIR}"

# Create mock kyber_aes.hpp
cat > "${HYDRA_CRYPTO_MOCK_DIR}/kyber_aes.hpp" << EOF
#pragma once

#include <vector>
#include <string>
#include <cstdint>

namespace hydra {
namespace crypto {

class KyberAES {
public:
    KyberAES() = default;
    ~KyberAES() = default;

    std::vector<uint8_t> generateKeypair(std::vector<uint8_t>& publicKey, std::vector<uint8_t>& privateKey) {
        // Mock implementation
        publicKey.resize(32, 0);
        privateKey.resize(32, 0);
        return publicKey;
    }

    std::vector<uint8_t> encapsulate(std::vector<uint8_t>& sharedKey, const std::vector<uint8_t>& publicKey) {
        // Mock implementation
        sharedKey.resize(32, 0);
        return std::vector<uint8_t>(32, 0);
    }

    void decapsulate(std::vector<uint8_t>& sharedKey, const std::vector<uint8_t>& ciphertext, const std::vector<uint8_t>& privateKey) {
        // Mock implementation
        sharedKey.resize(32, 0);
    }

    std::vector<uint8_t> encrypt(const std::vector<uint8_t>& plaintext, const std::vector<uint8_t>& key) {
        // Mock implementation
        return plaintext;
    }

    std::vector<uint8_t> decrypt(const std::vector<uint8_t>& ciphertext, const std::vector<uint8_t>& key) {
        // Mock implementation
        return ciphertext;
    }
};

} // namespace crypto
} // namespace hydra
EOF

# Create mock vector_crypto.hpp
cat > "${LMVS_MOCK_DIR}/security/vector_crypto.hpp" << EOF
#pragma once

#include <vector>
#include <string>
#include <cstdint>

namespace hydra {
namespace lmvs {
namespace security {

class VectorCrypto {
public:
    VectorCrypto() = default;
    ~VectorCrypto() = default;

    std::vector<uint8_t> encryptVector(const std::vector<uint8_t>& vector, const std::vector<uint8_t>& key) {
        // Mock implementation
        return vector;
    }

    std::vector<uint8_t> decryptVector(const std::vector<uint8_t>& encryptedVector, const std::vector<uint8_t>& key) {
        // Mock implementation
        return encryptedVector;
    }

    std::vector<uint8_t> generateKey() {
        // Mock implementation
        return std::vector<uint8_t>(32, 0);
    }
};

} // namespace security
} // namespace lmvs
} // namespace hydra
EOF

# Create mock secure_vector_transport.hpp
cat > "${LMVS_MOCK_DIR}/security/secure_vector_transport.hpp" << EOF
#pragma once

#include <vector>
#include <string>
#include <cstdint>
#include "vector_crypto.hpp"

namespace hydra {
namespace lmvs {
namespace security {

class SecureVectorTransport {
public:
    SecureVectorTransport() = default;
    ~SecureVectorTransport() = default;

    std::vector<uint8_t> encryptAndSign(const std::vector<uint8_t>& data, const std::vector<uint8_t>& key) {
        // Mock implementation
        return data;
    }

    std::vector<uint8_t> verifyAndDecrypt(const std::vector<uint8_t>& encryptedData, const std::vector<uint8_t>& key) {
        // Mock implementation
        return encryptedData;
    }
};

} // namespace security
} // namespace lmvs
} // namespace hydra
EOF

# Create mock p2p_secure_file.hpp
cat > "${LMVS_MOCK_DIR}/p2p_vfs/p2p_secure_file.hpp" << EOF
#pragma once

#include <vector>
#include <string>
#include <cstdint>
#include "../security/secure_vector_transport.hpp"

namespace hydra {
namespace lmvs {
namespace p2p_vfs {

class P2PSecureFile {
public:
    P2PSecureFile() = default;
    ~P2PSecureFile() = default;

    bool writeEncrypted(const std::string& path, const std::vector<uint8_t>& data, const std::vector<uint8_t>& key) {
        // Mock implementation
        return true;
    }

    std::vector<uint8_t> readEncrypted(const std::string& path, const std::vector<uint8_t>& key) {
        // Mock implementation
        return std::vector<uint8_t>();
    }
};

} // namespace p2p_vfs
} // namespace lmvs
} // namespace hydra
EOF

# Create mock p2p_vfs.hpp
cat > "${LMVS_MOCK_DIR}/p2p_vfs/p2p_vfs.hpp" << EOF
#pragma once

#include <vector>
#include <string>
#include <cstdint>
#include <map>
#include "p2p_secure_file.hpp"

namespace hydra {
namespace lmvs {
namespace p2p_vfs {

class P2PVFS {
public:
    P2PVFS() = default;
    ~P2PVFS() = default;

    bool addPeer(const std::string& peerId, const std::string& peerAddress) {
        // Mock implementation
        return true;
    }

    bool removePeer(const std::string& peerId) {
        // Mock implementation
        return true;
    }

    std::vector<std::string> getPeers() {
        // Mock implementation
        return std::vector<std::string>();
    }
};

} // namespace p2p_vfs
} // namespace lmvs
} // namespace hydra
EOF

echo "Mock dependencies for LMVS module have been created successfully!"
