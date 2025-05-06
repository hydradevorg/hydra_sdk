#include <iostream>
#include <complex>
#include <vector>
#include "hydra_qzkp/qzkp.hpp"

int main() {
    using namespace hydra::qzkp;

    QuantumZKP zkp(8, 128);

    std::vector<std::complex<double>> vec = {
        {0.6, 0.0}, {0.8, 0.0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}
    };

    std::string id = "example_user";

    auto [commitment, proof] = zkp.prove_vector_knowledge(vec, id);
    std::cout << "Commitment: ";
    for (auto b : commitment) std::cout << std::hex << (int)b;
    std::cout << "\n";

    std::cout << "Proof:\n" << proof.dump(2) << "\n";

    bool result = zkp.verify_proof(commitment, proof, id);
    std::cout << "\nProof verification: " << (result ? "✅ Success" : "❌ Failed") << "\n";

    return result ? 0 : 1;
}
