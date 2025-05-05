# Hydra Compression Library

## Overview

The `hydra_compression` library provides advanced data compression algorithms for the Hydra SDK. It includes specialized compression techniques optimized for different types of data, with a focus on efficiency and high compression ratios while maintaining data integrity.

## Key Components

### Optimal Subsequence Tokenization (OST)

The OST compression algorithm is designed for efficient compression of structured data by:

1. Dividing data into non-overlapping windows
2. Classifying these windows into bins based on similarity
3. Applying specialized compression strategies to each bin
4. Encoding the sequence of bin labels efficiently

Features:
- Configurable window length for optimal compression
- Pluggable classification strategies
- Customizable compression strategies for different data types
- Efficient label encoding using Huffman coding

### Transformer-based Video Compression (TVC)

The TVC system uses transformer-based models for video compression:

1. Tokenizes video frames
2. Builds context models for prediction
3. Uses transformer architecture to predict masked tokens
4. Quantizes predictions for efficient storage

Features:
- GOP (Group of Pictures) based compression
- Context-aware prediction for high compression ratios
- Transformer-based architecture for capturing temporal dependencies
- Quantization for size reduction

## Usage Examples

### OST Compression

```cpp
#include <hydra_compression/ost.h>
#include <vector>
#include <iostream>

using namespace hydra::compression;

int main() {
    // Create classification and compression strategies
    auto classifier = std::make_shared<DefaultClassificationStrategy>();
    auto compressor = std::make_shared<HuffmanCompressionStrategy>();
    
    // Create OST compressor with window length 1024 and label length 8
    OSTCompressor ost(classifier, compressor, 1024, 8);
    
    // Sample data to compress
    std::vector<uint8_t> data = {/* ... data ... */};
    
    // Compress the data
    std::vector<uint8_t> compressed = ost.compress(data);
    std::cout << "Original size: " << data.size() << " bytes" << std::endl;
    std::cout << "Compressed size: " << compressed.size() << " bytes" << std::endl;
    std::cout << "Compression ratio: " << (float)data.size() / compressed.size() << std::endl;
    
    // Decompress the data
    std::vector<uint8_t> decompressed = ost.decompress(compressed);
    
    // Verify the data is the same
    bool identical = (data == decompressed);
    std::cout << "Decompression successful: " << (identical ? "Yes" : "No") << std::endl;
    
    return 0;
}
```

### TVC Compression

```cpp
#include <hydra_compression/tvc/tvc.hpp>
#include <iostream>
#include <string>

using namespace hydra::compression;

int main() {
    // Create a TVC compressor with GOP size 12
    tvc compressor(12);
    
    // Compress a video file
    std::string input_path = "input_video.raw";
    std::string compressed_path = "compressed_video.tvc";
    
    std::cout << "Compressing video..." << std::endl;
    compressor.encode_video(input_path, compressed_path);
    
    // Decompress the video
    std::string output_path = "decompressed_video.raw";
    std::cout << "Decompressing video..." << std::endl;
    compressor.decode_video(compressed_path, output_path);
    
    return 0;
}
```

## Performance Considerations

- **Window Size**: Adjusting the window size in OST compression can significantly impact compression ratio and performance. Larger windows may capture more patterns but require more memory.
- **Classification Strategy**: The choice of classification strategy affects both compression ratio and computational complexity.
- **GOP Size**: In TVC, the GOP size determines the trade-off between compression ratio and random access capabilities.
- **Memory Usage**: Both algorithms are designed to be memory-efficient, but processing very large files may require streaming approaches.

## Integration with Hydra SDK

The `hydra_compression` library integrates with other components of the Hydra SDK:

- **hydra_math**: Leverages mathematical utilities like Huffman coding from the math library
- **hydra_vfs**: Can be used to compress files stored in the virtual file system
- **hydra_crypto**: Complements encrypted data with compression for efficient secure storage

## Dependencies

- C++20 compatible compiler
- Eigen library (for matrix operations in TVC)
- Google Test (for unit testing)

## Building

```bash
cd src/hydra_compression
mkdir build && cd build
cmake ..
make
```

## Testing

The library includes comprehensive tests to verify compression and decompression functionality:

```bash
cd build
ctest
```

## Future Developments

- Hardware acceleration support for compression operations
- Additional compression algorithms for specialized data types
- Adaptive compression strategies based on data characteristics
- Parallel processing for multi-core systems
