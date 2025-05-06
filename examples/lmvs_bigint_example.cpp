#include "lmvs/layered_bigint_vector.hpp"
#include "lmvs/layered_bigint_matrix.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <iomanip>
#include <fstream>

// Helper function to print a vector of doubles
void printVector(const std::vector<double>& vec) {
    std::cout << "[";
    for (size_t i = 0; i < vec.size(); ++i) {
        std::cout << std::fixed << std::setprecision(4) << vec[i];
        if (i < vec.size() - 1) {
            std::cout << ", ";
        }
    }
    std::cout << "]";
}

int main() {
    std::cout << "Layered Matrix and Vector System (LMVS) BigInt Example" << std::endl;
    std::cout << "=====================================================" << std::endl;
    
    // Create a layered BigInt vector from double values
    std::vector<std::vector<double>> data = {
        {1.0, 2.0, 3.0, 4.0},     // Layer 1
        {5.0, 6.0, 7.0, 8.0},     // Layer 2
        {9.0, 10.0, 11.0, 12.0}   // Layer 3
    };
    
    lmvs::LayeredBigIntVector vector(data);
    
    std::cout << "\n1. Created Layered BigInt Vector:" << std::endl;
    vector.print();
    
    // Create another layered BigInt vector
    std::vector<std::vector<double>> data2 = {
        {0.1, 0.2, 0.3, 0.4},     // Layer 1
        {0.5, 0.6, 0.7, 0.8},     // Layer 2
        {0.9, 1.0, 1.1, 1.2}      // Layer 3
    };
    
    lmvs::LayeredBigIntVector vector2(data2);
    
    // Create a layered BigInt matrix from the two vectors
    lmvs::LayeredBigIntMatrix matrix(vector, vector2);
    
    std::cout << "\n2. Created Layered BigInt Matrix from two vectors:" << std::endl;
    std::cout << "   (Only showing first block for brevity)" << std::endl;
    std::cout << "   Block [0][0]:" << std::endl;
    const auto& block = matrix.getBlock(0, 0);
    for (const auto& row : block) {
        std::cout << "   [";
        for (size_t i = 0; i < row.size(); ++i) {
            std::cout << row[i].to_string();
            if (i < row.size() - 1) {
                std::cout << ", ";
            }
        }
        std::cout << "]" << std::endl;
    }
    
    // Serialize and compress the vector
    std::vector<uint8_t> compressed = vector.compress();
    
    std::cout << "\n3. Compressed Vector:" << std::endl;
    std::cout << "   Original size: " << vector.serialize().size() << " bytes" << std::endl;
    std::cout << "   Compressed size: " << compressed.size() << " bytes" << std::endl;
    std::cout << "   Compression ratio: " << std::fixed << std::setprecision(2) 
              << (100.0 * compressed.size() / vector.serialize().size()) << "%" << std::endl;
    
    // Save compressed data to a file
    std::string compressed_file = "vector.lmvs";
    std::ofstream outfile(compressed_file, std::ios::binary);
    outfile.write(reinterpret_cast<const char*>(compressed.data()), compressed.size());
    outfile.close();
    
    std::cout << "   Saved compressed vector to " << compressed_file << std::endl;
    
    // Load compressed data from file
    std::ifstream infile(compressed_file, std::ios::binary);
    std::vector<uint8_t> loaded_data((std::istreambuf_iterator<char>(infile)), std::istreambuf_iterator<char>());
    infile.close();
    
    // Decompress the vector
    lmvs::LayeredBigIntVector decompressed_vector = lmvs::LayeredBigIntVector::decompress(loaded_data);
    
    std::cout << "\n4. Decompressed Vector:" << std::endl;
    decompressed_vector.print();
    
    // Convert back to double values
    std::vector<std::vector<double>> double_data = decompressed_vector.toDoubleVector();
    
    std::cout << "\n5. Converted back to double values:" << std::endl;
    for (size_t i = 0; i < double_data.size(); ++i) {
        std::cout << "   Layer " << i << ": ";
        printVector(double_data[i]);
        std::cout << std::endl;
    }
    
    // Serialize and compress the matrix
    std::vector<uint8_t> compressed_matrix = matrix.compress();
    
    std::cout << "\n6. Compressed Matrix:" << std::endl;
    std::cout << "   Original size: " << matrix.serialize().size() << " bytes" << std::endl;
    std::cout << "   Compressed size: " << compressed_matrix.size() << " bytes" << std::endl;
    std::cout << "   Compression ratio: " << std::fixed << std::setprecision(2) 
              << (100.0 * compressed_matrix.size() / matrix.serialize().size()) << "%" << std::endl;
    
    // Save compressed matrix to a file
    std::string compressed_matrix_file = "matrix.lmvs";
    std::ofstream outfile_matrix(compressed_matrix_file, std::ios::binary);
    outfile_matrix.write(reinterpret_cast<const char*>(compressed_matrix.data()), compressed_matrix.size());
    outfile_matrix.close();
    
    std::cout << "   Saved compressed matrix to " << compressed_matrix_file << std::endl;
    
    // Load compressed matrix from file
    std::ifstream infile_matrix(compressed_matrix_file, std::ios::binary);
    std::vector<uint8_t> loaded_matrix_data((std::istreambuf_iterator<char>(infile_matrix)), std::istreambuf_iterator<char>());
    infile_matrix.close();
    
    // Decompress the matrix
    lmvs::LayeredBigIntMatrix decompressed_matrix = lmvs::LayeredBigIntMatrix::decompress(loaded_matrix_data);
    
    std::cout << "\n7. Decompressed Matrix (first block):" << std::endl;
    const auto& decompressed_block = decompressed_matrix.getBlock(0, 0);
    for (const auto& row : decompressed_block) {
        std::cout << "   [";
        for (size_t i = 0; i < row.size(); ++i) {
            std::cout << row[i].to_string();
            if (i < row.size() - 1) {
                std::cout << ", ";
            }
        }
        std::cout << "]" << std::endl;
    }
    
    std::cout << "\nLMVS BigInt Example Completed Successfully!" << std::endl;
    
    return 0;
}
