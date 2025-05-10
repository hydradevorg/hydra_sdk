#!/bin/bash
# Script to integrate the GMP WebAssembly module with the LMVS module

# Set variables
HYDRA_ROOT="/volumes/bigcode/hydra_sdk"
WASM_MODULES_DIR="${HYDRA_ROOT}/wasm_modules"
LMVS_DIR="${WASM_MODULES_DIR}/lmvs"

# Create directories if they don't exist
mkdir -p "${LMVS_DIR}"

# Create a JavaScript wrapper for the LMVS module that uses the GMP WebAssembly module
echo "Creating JavaScript wrapper for LMVS module..."
cat > "${LMVS_DIR}/lmvs_wrapper.js" << 'EOF'
// LMVS WebAssembly wrapper
import { initGMPModule, createBigInt } from '../gmp/gmp_wrapper.js';
import { default as initLMVS } from './lmvs.js';

// Initialize the LMVS module
export async function initLMVSModule() {
  // First initialize the GMP module
  const gmp = await initGMPModule();
  
  // Then initialize the LMVS module
  const lmvs = await initLMVS({
    // Pass the GMP module to the LMVS module
    gmpModule: gmp
  });
  
  return {
    lmvs,
    gmp
  };
}

// BigInt Vector class
export class BigIntVector {
  constructor() {
    this._lmvs = null;
    this._gmp = null;
    this._vector = null;
    this._initialized = false;
  }
  
  async initialize(lmvsModule, gmpModule, size = 0) {
    if (this._initialized) return;
    
    this._lmvs = lmvsModule;
    this._gmp = gmpModule;
    
    // Create a new BigIntVector
    this._vector = new this._lmvs.BigIntVector(size);
    this._initialized = true;
  }
  
  // Get the size of the vector
  size() {
    if (!this._initialized) return 0;
    
    return this._vector.size();
  }
  
  // Set a value at a specific index
  async set(index, value) {
    if (!this._initialized) return;
    
    // Convert the value to a BigInt if it's not already
    let bigInt;
    if (typeof value === 'object' && value._mpz) {
      bigInt = value;
    } else {
      bigInt = await createBigInt(value, this._gmp);
    }
    
    this._vector.set(index, bigInt._mpz);
  }
  
  // Get a value at a specific index
  async get(index) {
    if (!this._initialized) return null;
    
    const mpz = this._vector.get(index);
    const bigInt = await createBigInt(0, this._gmp);
    
    // Copy the value from the vector to the BigInt
    this._gmp.mpz_set(bigInt._mpz, mpz);
    
    return bigInt;
  }
  
  // Convert to string
  toString() {
    if (!this._initialized) return "[]";
    
    return this._vector.toString();
  }
  
  // Clean up resources
  destroy() {
    if (this._initialized) {
      this._vector.delete();
      this._initialized = false;
    }
  }
}

// Layered BigInt Vector class
export class LayeredBigIntVector {
  constructor() {
    this._lmvs = null;
    this._gmp = null;
    this._vector = null;
    this._initialized = false;
  }
  
  async initialize(lmvsModule, gmpModule, numLayers = 1, size = 0) {
    if (this._initialized) return;
    
    this._lmvs = lmvsModule;
    this._gmp = gmpModule;
    
    // Create a new LayeredBigIntVector
    this._vector = new this._lmvs.LayeredBigIntVector(numLayers, size);
    this._initialized = true;
  }
  
  // Get the number of layers
  numLayers() {
    if (!this._initialized) return 0;
    
    return this._vector.numLayers();
  }
  
  // Get the size of the vector
  size() {
    if (!this._initialized) return 0;
    
    return this._vector.size();
  }
  
  // Set a value at a specific layer and index
  async set(layer, index, value) {
    if (!this._initialized) return;
    
    // Convert the value to a BigInt if it's not already
    let bigInt;
    if (typeof value === 'object' && value._mpz) {
      bigInt = value;
    } else {
      bigInt = await createBigInt(value, this._gmp);
    }
    
    this._vector.set(layer, index, bigInt._mpz);
  }
  
  // Get a value at a specific layer and index
  async get(layer, index) {
    if (!this._initialized) return null;
    
    const mpz = this._vector.get(layer, index);
    const bigInt = await createBigInt(0, this._gmp);
    
    // Copy the value from the vector to the BigInt
    this._gmp.mpz_set(bigInt._mpz, mpz);
    
    return bigInt;
  }
  
  // Convert to string
  toString() {
    if (!this._initialized) return "[]";
    
    return this._vector.toString();
  }
  
  // Clean up resources
  destroy() {
    if (this._initialized) {
      this._vector.delete();
      this._initialized = false;
    }
  }
}

// Create factory functions
export async function createBigIntVector(lmvsModule, gmpModule, size = 0) {
  const vector = new BigIntVector();
  await vector.initialize(lmvsModule, gmpModule, size);
  return vector;
}

export async function createLayeredBigIntVector(lmvsModule, gmpModule, numLayers = 1, size = 0) {
  const vector = new LayeredBigIntVector();
  await vector.initialize(lmvsModule, gmpModule, numLayers, size);
  return vector;
}
EOF

# Create a test HTML file for the LMVS module
echo "Creating test HTML file for LMVS module..."
cat > "${WASM_MODULES_DIR}/lmvs_test.html" << 'EOF'
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>LMVS WebAssembly Test</title>
  <style>
    body {
      font-family: Arial, sans-serif;
      margin: 20px;
      line-height: 1.6;
    }
    h1 {
      color: #333;
    }
    pre {
      background-color: #f5f5f5;
      padding: 10px;
      border-radius: 5px;
      overflow-x: auto;
    }
    button {
      padding: 8px 16px;
      background-color: #4CAF50;
      color: white;
      border: none;
      border-radius: 4px;
      cursor: pointer;
      margin-right: 10px;
      margin-bottom: 10px;
    }
    button:hover {
      background-color: #45a049;
    }
    #output {
      margin-top: 20px;
      white-space: pre-wrap;
    }
  </style>
</head>
<body>
  <h1>LMVS WebAssembly Test</h1>
  
  <button id="testBigIntVector">Test BigIntVector</button>
  <button id="testLayeredBigIntVector">Test LayeredBigIntVector</button>
  <button id="clearOutput">Clear Output</button>
  
  <pre id="output"></pre>

  <script type="module">
    import { initLMVSModule, createBigIntVector, createLayeredBigIntVector } from './lmvs/lmvs_wrapper.js';
    import { createBigInt } from './gmp/gmp_wrapper.js';
    
    const outputElement = document.getElementById('output');
    
    function log(message) {
      outputElement.textContent += message + '\n';
    }
    
    document.getElementById('clearOutput').addEventListener('click', () => {
      outputElement.textContent = '';
    });
    
    document.getElementById('testBigIntVector').addEventListener('click', async () => {
      try {
        log('Initializing LMVS module...');
        const { lmvs, gmp } = await initLMVSModule();
        log('LMVS module initialized successfully!');
        
        log('\nTesting BigIntVector:');
        
        // Create a BigIntVector
        const vector = await createBigIntVector(lmvs, gmp, 5);
        log(`Created BigIntVector with size: ${vector.size()}`);
        
        // Set values
        await vector.set(0, "12345678901234567890");
        await vector.set(1, "98765432109876543210");
        await vector.set(2, 42);
        await vector.set(3, 123);
        await vector.set(4, 456);
        
        log(`Vector: ${vector.toString()}`);
        
        // Get values
        const value0 = await vector.get(0);
        const value1 = await vector.get(1);
        const value2 = await vector.get(2);
        
        log(`Value at index 0: ${value0.toString()}`);
        log(`Value at index 1: ${value1.toString()}`);
        log(`Value at index 2: ${value2.toString()}`);
        
        // Clean up
        vector.destroy();
        value0.destroy();
        value1.destroy();
        value2.destroy();
        
        log('\nBigIntVector test completed successfully!');
      } catch (error) {
        log(`Error: ${error.message}`);
        console.error(error);
      }
    });
    
    document.getElementById('testLayeredBigIntVector').addEventListener('click', async () => {
      try {
        log('Initializing LMVS module...');
        const { lmvs, gmp } = await initLMVSModule();
        log('LMVS module initialized successfully!');
        
        log('\nTesting LayeredBigIntVector:');
        
        // Create a LayeredBigIntVector with 3 layers and 4 elements
        const vector = await createLayeredBigIntVector(lmvs, gmp, 3, 4);
        log(`Created LayeredBigIntVector with ${vector.numLayers()} layers and size: ${vector.size()}`);
        
        // Set values in different layers
        await vector.set(0, 0, "12345678901234567890");
        await vector.set(0, 1, "98765432109876543210");
        await vector.set(1, 0, 42);
        await vector.set(1, 1, 123);
        await vector.set(2, 0, 456);
        await vector.set(2, 1, 789);
        
        log(`Vector: ${vector.toString()}`);
        
        // Get values from different layers
        const value00 = await vector.get(0, 0);
        const value01 = await vector.get(0, 1);
        const value10 = await vector.get(1, 0);
        const value20 = await vector.get(2, 0);
        
        log(`Value at layer 0, index 0: ${value00.toString()}`);
        log(`Value at layer 0, index 1: ${value01.toString()}`);
        log(`Value at layer 1, index 0: ${value10.toString()}`);
        log(`Value at layer 2, index 0: ${value20.toString()}`);
        
        // Clean up
        vector.destroy();
        value00.destroy();
        value01.destroy();
        value10.destroy();
        value20.destroy();
        
        log('\nLayeredBigIntVector test completed successfully!');
      } catch (error) {
        log(`Error: ${error.message}`);
        console.error(error);
      }
    });
    
    log('LMVS WebAssembly test page loaded. Click the buttons to run tests.');
  </script>
</body>
</html>
EOF

echo "Integration complete!"
echo "Now open http://localhost:8000/lmvs_test.html in your browser to test the LMVS module"
