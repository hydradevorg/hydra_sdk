#include "hydra_compression/tvc/tvc.hpp"
#include "hydra_compression/tvc/tokenizer.hpp"
#include "hydra_compression/tvc/context_model.hpp"
#include "hydra_compression/tvc/transformer.hpp"
#include "hydra_compression/tvc/quantizer.hpp"
#include "hydra_compression/tvc/decoder.hpp"

#include <fstream>
#include <iostream>
#include <sstream>


namespace hydra { namespace compression {

tvc::tvc(int gop_size) : gop_size(gop_size) {}

void tvc::encode_video(const std::string& input_path, const std::string& output_path) {
    std::ifstream infile(input_path);
    std::ofstream outfile(output_path);
    if (!infile || !outfile) {
        std::cerr << "File error\n";
        return;
    }

    tokenizer t;
    context_model cm;
    transformer tf;
    quantizer q;

    std::string line;
    int frame_index = 0;

    while (std::getline(infile, line)) {
        auto tokens = t.tokenize(line);
        cm.train(tokens);

        std::vector<std::string> masked = tokens;
        if (!masked.empty()) masked[0] = "[MASK]";  // simulate loss

        auto predicted = tf.predict_masked_tokens(masked, cm.get_probabilities());
        auto quantized = q.quantize(predicted);

        for (int idx : quantized) {
            outfile << idx << ' ';
        }
        outfile << '\n';

        ++frame_index;
    }
}

void tvc::decode_video(const std::string& input_path, const std::string& output_path) {
    std::ifstream infile(input_path);
    std::ofstream outfile(output_path);
    if (!infile || !outfile) {
        std::cerr << "File error\n";
        return;
    }

    quantizer q;
    decoder d;

    std::string line;
    int frame_index = 0;

    while (std::getline(infile, line)) {
        std::istringstream iss(line);
        std::vector<int> indices;
        int val;
        while (iss >> val) {
            indices.push_back(val);
        }

        auto tokens = q.dequantize(indices);
        std::string frame = d.reconstruct_frame(tokens, frame_index++);
        outfile << frame << '\n';
    }
}

}} // namespace hydra::compression
