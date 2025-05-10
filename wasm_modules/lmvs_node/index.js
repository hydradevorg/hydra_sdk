const bigInt = require('big-integer');

/**
 * Layered Matrix Vector System (LMVS) implementation
 * This class implements the core functionality of the LMVS as described in the academic paper
 */
class LayeredMatrixVectorSystem {
  /**
   * Create a new LMVS instance
   * @param {number} numLayers - Number of layers in the system
   * @param {number} vectorSize - Size of vectors in the system
   */
  constructor(numLayers = 3, vectorSize = 32) {
    this.numLayers = numLayers;
    this.vectorSize = vectorSize;
    
    // Initialize matrices with random values
    this.matrices = [];
    for (let i = 0; i < numLayers; i++) {
      const matrix = [];
      for (let j = 0; j < vectorSize; j++) {
        const row = [];
        for (let k = 0; k < vectorSize; k++) {
          // Use bigInt for all values to handle large numbers
          row.push(bigInt.randBetween(1, 1000));
        }
        matrix.push(row);
      }
      this.matrices.push(matrix);
    }
    
    // Initialize vectors with random values
    this.vectors = [];
    for (let i = 0; i < numLayers; i++) {
      const vector = [];
      for (let j = 0; j < vectorSize; j++) {
        vector.push(bigInt.randBetween(1, 1000));
      }
      this.vectors.push(vector);
    }
  }
  
  /**
   * Generate a random vector of the appropriate size
   * @returns {Array} - A random vector
   */
  generateVector() {
    const vector = [];
    for (let i = 0; i < this.vectorSize; i++) {
      vector.push(bigInt.randBetween(1, 1000));
    }
    return vector;
  }
  
  /**
   * Multiply a vector by the layered matrices
   * @param {Array} inputVector - The input vector to multiply
   * @returns {Array} - The resulting vector after multiplication
   */
  multiplyVector(inputVector) {
    if (inputVector.length !== this.vectorSize) {
      throw new Error(`Input vector size (${inputVector.length}) does not match system vector size (${this.vectorSize})`);
    }
    
    // Convert input to bigInt if needed
    let result = inputVector.map(val => typeof val === 'object' ? val : bigInt(val));
    
    // Apply each layer
    for (let layer = 0; layer < this.numLayers; layer++) {
      const layerResult = Array(this.vectorSize).fill(bigInt.zero);
      
      // Matrix multiplication
      for (let i = 0; i < this.vectorSize; i++) {
        for (let j = 0; j < this.vectorSize; j++) {
          layerResult[i] = layerResult[i].add(
            this.matrices[layer][i][j].multiply(result[j])
          );
        }
      }
      
      // Add the layer vector
      for (let i = 0; i < this.vectorSize; i++) {
        layerResult[i] = layerResult[i].add(this.vectors[layer][i]);
      }
      
      result = layerResult;
    }
    
    return result;
  }
  
  /**
   * Compress a vector using a simple compression algorithm
   * @param {Array} vector - The vector to compress
   * @returns {Uint8Array} - The compressed vector
   */
  compressVector(vector) {
    // Convert bigInt values to strings
    const stringValues = vector.map(val => val.toString());
    
    // Join with a delimiter
    const joinedString = stringValues.join(',');
    
    // Convert to UTF-8 encoded bytes
    const encoder = new TextEncoder();
    return encoder.encode(joinedString);
  }
  
  /**
   * Decompress a vector that was compressed with compressVector
   * @param {Uint8Array} compressedVector - The compressed vector
   * @returns {Array} - The decompressed vector
   */
  decompressVector(compressedVector) {
    // Convert bytes to string
    const decoder = new TextDecoder();
    const joinedString = decoder.decode(compressedVector);
    
    // Split by delimiter and convert back to bigInt
    return joinedString.split(',').map(val => bigInt(val));
  }
  
  /**
   * Get a string representation of this LMVS
   * @returns {string} - A string describing this LMVS
   */
  toString() {
    return `LMVS with ${this.numLayers} layers and vector size ${this.vectorSize}`;
  }
}

// Export the module
module.exports = {
  LayeredMatrixVectorSystem,
  
  // API functions
  createLMVS: function(numLayers, vectorSize) {
    return new LayeredMatrixVectorSystem(numLayers, vectorSize);
  },
  
  getLMVSNumLayers: function(lmvs) {
    return lmvs.numLayers;
  },
  
  getLMVSVectorSize: function(lmvs) {
    return lmvs.vectorSize;
  },
  
  generateLMVSVector: function(lmvs) {
    return lmvs.generateVector();
  },
  
  multiplyLMVSVector: function(lmvs, vector) {
    return lmvs.multiplyVector(vector);
  },
  
  compressLMVSVector: function(lmvs, vector) {
    return lmvs.compressVector(vector);
  },
  
  decompressLMVSVector: function(lmvs, compressedVector) {
    return lmvs.decompressVector(compressedVector);
  },
  
  getLMVSString: function(lmvs) {
    return lmvs.toString();
  },
  
  destroyLMVS: function(lmvs) {
    // JavaScript has garbage collection, so no explicit destruction is needed
  }
};
