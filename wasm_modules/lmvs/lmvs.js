// LMVS Module - JavaScript Implementation with BigInt support

// Simple BigInt implementation for browsers that don't support native BigInt
class BigInteger {
  constructor(value) {
    this.value = typeof value === 'number' ? value : parseInt(value, 10);
  }

  add(other) {
    return new BigInteger(this.value + (typeof other === 'object' && 'value' in other ? other.value : other));
  }

  multiply(other) {
    return new BigInteger(this.value * (typeof other === 'object' && 'value' in other ? other.value : other));
  }

  toString() {
    return this.value.toString();
  }

  valueOf() {
    return this.value;
  }

  static zero = new BigInteger(0);

  static randBetween(min, max) {
    const minVal = typeof min === 'object' && 'value' in min ? min.value : min;
    const maxVal = typeof max === 'object' && 'value' in max ? max.value : max;
    return new BigInteger(Math.floor(Math.random() * (maxVal - minVal + 1)) + minVal);
  }
}

// Use native BigInt if available, otherwise use our implementation
const bigInt = typeof BigInt !== 'undefined' ?
  (val) => typeof val === 'bigint' ? val : BigInt(Number(val)) :
  (val) => new BigInteger(val);

bigInt.zero = typeof BigInt !== 'undefined' ? BigInt(0) : BigInteger.zero;
bigInt.randBetween = (min, max) => {
  if (typeof BigInt !== 'undefined') {
    const minVal = typeof min === 'bigint' ? min : BigInt(Number(min));
    const maxVal = typeof max === 'bigint' ? max : BigInt(Number(max));
    const range = maxVal - minVal + BigInt(1);
    const randomBigInt = minVal + BigInt(Math.floor(Math.random() * Number(range)));
    return randomBigInt;
  } else {
    return BigInteger.randBetween(min, max);
  }
};

class LayeredMatrixVectorSystem {
  constructor(numLayers = 3, vectorSize = 32) {
    this.numLayers = numLayers;
    this.vectorSize = vectorSize;

    // Initialize matrices with random values using BigInt
    this.matrices = [];
    for (let i = 0; i < numLayers; i++) {
      const matrix = [];
      for (let j = 0; j < vectorSize; j++) {
        const row = [];
        for (let k = 0; k < vectorSize; k++) {
          row.push(bigInt.randBetween(1, 100));
        }
        matrix.push(row);
      }
      this.matrices.push(matrix);
    }

    // Initialize vectors with random values using BigInt
    this.vectors = [];
    for (let i = 0; i < numLayers; i++) {
      const vector = [];
      for (let j = 0; j < vectorSize; j++) {
        vector.push(bigInt.randBetween(1, 100));
      }
      this.vectors.push(vector);
    }
  }

  createMatrix(rows, cols) {
    const matrix = {
      rows: rows,
      cols: cols,
      data: Array(rows * cols).fill(0),

      get: function(row, col) {
        return this.data[row * this.cols + col];
      },

      set: function(row, col, value) {
        this.data[row * this.cols + col] = value;
      }
    };

    return matrix;
  }

  createVector(size) {
    const vector = {
      size: size,
      data: Array(size).fill(0),

      get: function(index) {
        return this.data[index];
      },

      set: function(index, value) {
        this.data[index] = value;
      }
    };

    return vector;
  }

  multiply(matrix, vector) {
    if (matrix.cols !== vector.size) {
      throw new Error("Matrix columns must match vector size");
    }

    const result = this.createVector(matrix.rows);

    for (let i = 0; i < matrix.rows; i++) {
      let sum = 0;
      for (let j = 0; j < matrix.cols; j++) {
        sum += matrix.get(i, j) * vector.get(j);
      }
      result.set(i, sum);
    }

    return result;
  }

  generateVector() {
    const vector = [];
    for (let i = 0; i < this.vectorSize; i++) {
      vector.push(bigInt.randBetween(1, 100));
    }
    return vector;
  }

  multiplyVector(inputVector) {
    if (inputVector.length !== this.vectorSize) {
      throw new Error("Input vector size does not match system vector size");
    }

    // Convert input to bigInt if needed
    let result = inputVector.map(val => {
      if (typeof val === 'object' && ('value' in val || typeof val === 'bigint')) {
        return val;
      } else {
        return bigInt(val);
      }
    });

    // Apply each layer
    for (let layer = 0; layer < this.numLayers; layer++) {
      const layerResult = Array(this.vectorSize).fill(bigInt.zero);

      // Matrix multiplication
      for (let i = 0; i < this.vectorSize; i++) {
        for (let j = 0; j < this.vectorSize; j++) {
          // Handle different types of BigInt implementations
          if (typeof BigInt !== 'undefined') {
            layerResult[i] += this.matrices[layer][i][j] * result[j];
          } else {
            layerResult[i] = layerResult[i].add(
              this.matrices[layer][i][j].multiply(result[j])
            );
          }
        }
      }

      // Add the layer vector
      for (let i = 0; i < this.vectorSize; i++) {
        if (typeof BigInt !== 'undefined') {
          layerResult[i] += this.vectors[layer][i];
        } else {
          layerResult[i] = layerResult[i].add(this.vectors[layer][i]);
        }
      }

      result = layerResult;
    }

    return result;
  }

  toString() {
    return `LMVS with ${this.numLayers} layers and vector size ${this.vectorSize}`;
  }
}

// Export the module
export default function() {
  return {
    LayeredMatrixVectorSystem: LayeredMatrixVectorSystem,

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

    getLMVSString: function(lmvs) {
      return lmvs.toString();
    },

    destroyLMVS: function(lmvs) {
      // JavaScript has garbage collection, so no explicit destruction is needed
    }
  };
}
