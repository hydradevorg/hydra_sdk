#pragma once

#include "command.h"
#include <hydra_crypto/kyber_kem.hpp>
#include <hydra_crypto/dilithium_signature.hpp>
#include <hydra_crypto/falcon_signature.hpp>
#include <memory>
#include <string>
#include <vector>

namespace hydra {
namespace cli {
namespace crypto {

// Command to generate cryptographic keys
class KeygenCommand : public Command {
public:
    int execute(const std::vector<std::string>& args) override;
    void print_help() const override;

private:
    // Methods for each algorithm
    int generate_kyber_keys(const std::vector<std::string>& args);
    int generate_dilithium_keys(const std::vector<std::string>& args);
    int generate_falcon_keys(const std::vector<std::string>& args);
};

// Command to encrypt data
class EncryptCommand : public Command {
public:
    int execute(const std::vector<std::string>& args) override;
    void print_help() const override;

private:
    // Methods for each encryption algorithm
    int encrypt_with_kyber(const std::vector<std::string>& args);
    int encrypt_file(const std::vector<std::string>& args);
};

// Command to decrypt data
class DecryptCommand : public Command {
public:
    int execute(const std::vector<std::string>& args) override;
    void print_help() const override;

private:
    // Methods for each decryption algorithm
    int decrypt_with_kyber(const std::vector<std::string>& args);
    int decrypt_file(const std::vector<std::string>& args);
};

// Command to sign data
class SignCommand : public Command {
public:
    int execute(const std::vector<std::string>& args) override;
    void print_help() const override;

private:
    // Methods for each signature algorithm
    int sign_with_dilithium(const std::vector<std::string>& args);
    int sign_with_falcon(const std::vector<std::string>& args);
    int sign_file(const std::vector<std::string>& args);
};

// Command to verify signatures
class VerifyCommand : public Command {
public:
    int execute(const std::vector<std::string>& args) override;
    void print_help() const override;

private:
    // Methods for each verification algorithm
    int verify_with_dilithium(const std::vector<std::string>& args);
    int verify_with_falcon(const std::vector<std::string>& args);
    int verify_file(const std::vector<std::string>& args);
};

// Command to display information about a key
class KeyInfoCommand : public Command {
public:
    int execute(const std::vector<std::string>& args) override;
    void print_help() const override;
};

} // namespace crypto
} // namespace cli
} // namespace hydra
