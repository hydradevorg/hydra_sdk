#include <gtest/gtest.h>
#include "hydra_compression/tvc/tvc.hpp"

#include <fstream>
#include <sstream>

// Filesystem fallback for libc++ / Clang
#if __has_include(<filesystem>)
  #include <filesystem>
  namespace fs = std::filesystem;
#elif __has_include(<experimental/filesystem>)
  #include <experimental/filesystem>
  namespace fs = std::experimental::filesystem;
#else
  #error "No <filesystem> support"
#endif

using namespace hydra::compression;
TEST(TVC, EncodeDecodeRoundtrip) {
    const std::string input_path = "test_input.txt";
    const std::string encoded_path = "test_encoded.tvc";
    const std::string decoded_path = "test_output.txt";

    // Create dummy input
    std::ofstream input(input_path);
    input << "hello world this is a test\n";
    input << "another test line to encode\n";
    input.close();

    // Run encode
    tvc compressor;
    compressor.encode_video(input_path, encoded_path);

    // Check output file exists
    ASSERT_TRUE(fs::exists(encoded_path));
    ASSERT_GT(fs::file_size(encoded_path), 0);

    // Run decode
    compressor.decode_video(encoded_path, decoded_path);

    // Check output
    std::ifstream output(decoded_path);
    std::string line;
    int lines = 0;
    while (std::getline(output, line)) {
        ASSERT_TRUE(line.find("Frame") != std::string::npos);
        ++lines;
    }

    EXPECT_EQ(lines, 2);

    // Cleanup
    fs::remove(input_path);
    fs::remove(encoded_path);
    fs::remove(decoded_path);
}
