#include <emscripten/bind.h>
#include <string>
#include <vector>
#include <memory>

namespace hydra {
namespace lmvs {

// Forward declaration of the C API functions
extern "C" {
    void lmvs_init(int numLayers, int vectorSize);
    int lmvs_get_num_layers();
    int lmvs_get_vector_size();
    int* lmvs_generate_vector();
    int* lmvs_multiply_vector(const int* inputVector, int size);
}

// C++ wrapper class for the JavaScript bindings
class LMVS {
public:
    LMVS(int numLayers = 3, int vectorSize = 32) {
        lmvs_init(numLayers, vectorSize);
    }
    
    int getNumLayers() const {
        return lmvs_get_num_layers();
    }
    
    int getVectorSize() const {
        return lmvs_get_vector_size();
    }
    
    std::vector<int> generateVector() {
        int* ptr = lmvs_generate_vector();
        if (!ptr) {
            return std::vector<int>();
        }
        
        int size = getVectorSize();
        return std::vector<int>(ptr, ptr + size);
    }
    
    std::vector<int> multiplyVector(const std::vector<int>& inputVector) {
        int* ptr = lmvs_multiply_vector(inputVector.data(), inputVector.size());
        if (!ptr) {
            return std::vector<int>();
        }
        
        int size = getVectorSize();
        return std::vector<int>(ptr, ptr + size);
    }
    
    std::string toString() const {
        return "LMVS with " + std::to_string(getNumLayers()) + " layers and vector size " + std::to_string(getVectorSize());
    }
};

// Helper function to convert a JavaScript array to a C++ vector
std::vector<int> jsArrayToVector(const emscripten::val& jsArray) {
    std::vector<int> result;
    const auto length = jsArray["length"].as<unsigned>();
    result.reserve(length);
    
    for (unsigned i = 0; i < length; ++i) {
        result.push_back(jsArray[i].as<int>());
    }
    
    return result;
}

// Helper function to convert a C++ vector to a JavaScript array
emscripten::val vectorToJsArray(const std::vector<int>& vec) {
    auto result = emscripten::val::array();
    for (size_t i = 0; i < vec.size(); ++i) {
        result.set(i, vec[i]);
    }
    return result;
}

// Wrapper functions for JavaScript
LMVS* createLMVS(int numLayers, int vectorSize) {
    return new LMVS(numLayers, vectorSize);
}

int getLMVSNumLayers(LMVS* lmvs) {
    return lmvs->getNumLayers();
}

int getLMVSVectorSize(LMVS* lmvs) {
    return lmvs->getVectorSize();
}

emscripten::val generateLMVSVector(LMVS* lmvs) {
    return vectorToJsArray(lmvs->generateVector());
}

emscripten::val multiplyLMVSVector(LMVS* lmvs, const emscripten::val& jsArray) {
    std::vector<int> inputVector = jsArrayToVector(jsArray);
    return vectorToJsArray(lmvs->multiplyVector(inputVector));
}

std::string getLMVSString(LMVS* lmvs) {
    return lmvs->toString();
}

void destroyLMVS(LMVS* lmvs) {
    delete lmvs;
}

// Register the bindings
EMSCRIPTEN_BINDINGS(lmvs_module) {
    emscripten::function("createLMVS", &createLMVS, emscripten::allow_raw_pointers());
    emscripten::function("getLMVSNumLayers", &getLMVSNumLayers, emscripten::allow_raw_pointers());
    emscripten::function("getLMVSVectorSize", &getLMVSVectorSize, emscripten::allow_raw_pointers());
    emscripten::function("generateLMVSVector", &generateLMVSVector, emscripten::allow_raw_pointers());
    emscripten::function("multiplyLMVSVector", &multiplyLMVSVector, emscripten::allow_raw_pointers());
    emscripten::function("getLMVSString", &getLMVSString, emscripten::allow_raw_pointers());
    emscripten::function("destroyLMVS", &destroyLMVS, emscripten::allow_raw_pointers());
}

} // namespace lmvs
} // namespace hydra
