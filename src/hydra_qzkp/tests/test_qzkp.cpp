#include <gtest/gtest.h>
#include "hydra_qzkp/qzkp.hpp"

using namespace hydra::qzkp;

TEST(QuantumZKPTest, ProveAndVerify) {
    QuantumZKP zkp(8, 128);

    std::vector<std::complex<double>> vector(8);
    vector[0] = 0.6;
    vector[1] = 0.8;  // normalized: 0.36 + 0.64 = 1.0

    auto [commitment, proof] = zkp.prove_vector_knowledge(vector, "test_id");
    EXPECT_FALSE(proof.empty());

    bool verified = zkp.verify_proof(commitment, proof, "test_id");
    EXPECT_TRUE(verified);
}
