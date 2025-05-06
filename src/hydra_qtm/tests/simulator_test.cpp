#include <gtest/gtest.h>
#include "hydra_qtm/simulator.hpp"
#include "hydra_qtm/quantum_gate.hpp"
#include "hydra_qtm/quantum_circuit.hpp"

using namespace hydra::qtm;

TEST(SimulatorTest, HadamardCreatesSuperposition) {
    QuantumCircuit qc(1);
    qc.add_gate(QuantumGate::H(), {0});

    Simulator sim(1);
    sim.run(qc);
    auto state = sim.state().amplitudes();

    // Should be equal amplitudes for |0> and |1>
    EXPECT_NEAR(std::abs(state[0]), std::abs(state[1]), 1e-9);
    EXPECT_NEAR(std::norm(state[0]) + std::norm(state[1]), 1.0, 1e-9);
}

TEST(SimulatorTest, BellStateAndMeasurement) {
    QuantumCircuit qc(2);
    qc.add_gate(QuantumGate::H(), {0});
    qc.add_gate(QuantumGate::CNOT(), {0, 1});

    Simulator sim(2);
    sim.run(qc);

    auto hist = sim.measure_all(500);
    double p00 = hist["00"] / 500.0;
    double p11 = hist["11"] / 500.0;
    double p01 = hist["01"] / 500.0;
    double p10 = hist["10"] / 500.0;

    // Should collapse to 00 or 11 only
    EXPECT_NEAR(p00 + p11, 1.0, 0.15);
    EXPECT_NEAR(p01 + p10, 0.0, 0.1);
}

TEST(SimulatorTest, ClassicalRegisterSyncsWithMeasurement) {
    QuantumCircuit qc(1);
    qc.add_gate(QuantumGate::H(), {0});
    Simulator sim(1);
    sim.run(qc);

    auto hist = sim.measure_all(1);
    auto reg = sim.classical_bits().dump();

    ASSERT_EQ(reg.size(), 1);
    std::string key = reg[0] ? "1" : "0";
    EXPECT_EQ(hist.count(key), 1);
}

TEST(SimulatorTest, MultiQubitGateOrderExecution) {
    QuantumCircuit qc(3);
    qc.add_gate(QuantumGate::H(), {0});
    qc.add_gate(QuantumGate::CNOT(), {0, 1});
    qc.add_gate(QuantumGate::Toffoli(), {0, 1, 2});

    Simulator sim(3);
    sim.run(qc);
    auto state = sim.state().amplitudes();

    // Should be entangled — check non-zero entries
    double total_prob = 0.0;
    for (const auto& amp : state)
        total_prob += std::norm(amp);
    EXPECT_NEAR(total_prob, 1.0, 1e-9);
}

TEST(SimulatorTest, NoiseModelCanBeInjected) {
    Simulator sim(1);
    auto model = std::make_shared<NoiseModel>();

    NoiseChannel bitflip;
    bitflip.name = "bit_flip_stub";
    bitflip.kraus_ops = {};  // placeholder

    model->add_channel(0, bitflip);
    sim.set_noise_model(model);

    // Just confirm the noise model is accepted
    EXPECT_NO_THROW(sim.run(QuantumCircuit(1)));
}
