#!/bin/bash
# Script to fix the WebAssembly build process and generate JavaScript bindings

# Set variables
HYDRA_ROOT="/Volumes/BIGCODE/hydra_sdk"
MOCK_DIR="${HYDRA_ROOT}/build_wasm/mock"
GMP_WASM_DIR="${HYDRA_ROOT}/gmp-wasm"
WASM_LIB_DIR="${HYDRA_ROOT}/lib/wasm"
WASM_MODULES_DIR="${HYDRA_ROOT}/wasm_modules"

# Create directories if they don't exist
mkdir -p "${MOCK_DIR}/include"
mkdir -p "${MOCK_DIR}/lib"
mkdir -p "${WASM_LIB_DIR}/include"
mkdir -p "${WASM_LIB_DIR}/lib"
mkdir -p "${WASM_MODULES_DIR}/gmp"

# Copy GMP WebAssembly files to the wasm_modules directory
echo "Copying GMP WebAssembly files..."
cp -f "${GMP_WASM_DIR}/binding/dist/gmp.js" "${WASM_MODULES_DIR}/gmp/"
cp -f "${GMP_WASM_DIR}/binding/dist/gmp.wasm" "${WASM_MODULES_DIR}/gmp/"
cp -f "${GMP_WASM_DIR}/binding/dist/gmpmini.js" "${WASM_MODULES_DIR}/gmp/"
cp -f "${GMP_WASM_DIR}/binding/dist/gmpmini.wasm" "${WASM_MODULES_DIR}/gmp/"

# Create a JavaScript wrapper for the GMP WebAssembly module
echo "Creating JavaScript wrapper for GMP WebAssembly module..."
cat > "${WASM_MODULES_DIR}/gmp/gmp_wrapper.js" << 'EOF'
// GMP WebAssembly wrapper
import { default as initGMP } from './gmp.js';

// Initialize the GMP module
export async function initGMPModule() {
  const gmp = await initGMP();
  return gmp;
}

// Create a BigInt class that uses the GMP WebAssembly module
export class BigInt {
  constructor(value = 0) {
    this._gmp = null;
    this._mpz = null;
    this._initialized = false;
    
    // Store the initial value to set after initialization
    this._initialValue = value;
  }
  
  async initialize(gmpModule) {
    if (this._initialized) return;
    
    this._gmp = gmpModule;
    this._mpz = new this._gmp.mpz_t();
    this._gmp.mpz_init(this._mpz);
    
    // Set the initial value
    if (typeof this._initialValue === 'number') {
      this._gmp.mpz_set_si(this._mpz, this._initialValue);
    } else if (typeof this._initialValue === 'string') {
      this._gmp.mpz_set_str(this._mpz, this._initialValue, 10);
    }
    
    this._initialized = true;
  }
  
  // Convert to string
  toString(base = 10) {
    if (!this._initialized) return "0";
    
    const str = this._gmp.mpz_get_str(null, base, this._mpz);
    return this._gmp.UTF8ToString(str);
  }
  
  // Arithmetic operations
  async add(other) {
    if (!this._initialized || !other._initialized) return new BigInt(0);
    
    const result = new BigInt(0);
    await result.initialize(this._gmp);
    
    this._gmp.mpz_add(result._mpz, this._mpz, other._mpz);
    return result;
  }
  
  async subtract(other) {
    if (!this._initialized || !other._initialized) return new BigInt(0);
    
    const result = new BigInt(0);
    await result.initialize(this._gmp);
    
    this._gmp.mpz_sub(result._mpz, this._mpz, other._mpz);
    return result;
  }
  
  async multiply(other) {
    if (!this._initialized || !other._initialized) return new BigInt(0);
    
    const result = new BigInt(0);
    await result.initialize(this._gmp);
    
    this._gmp.mpz_mul(result._mpz, this._mpz, other._mpz);
    return result;
  }
  
  async divide(other) {
    if (!this._initialized || !other._initialized) return new BigInt(0);
    
    const result = new BigInt(0);
    await result.initialize(this._gmp);
    
    this._gmp.mpz_fdiv_q(result._mpz, this._mpz, other._mpz);
    return result;
  }
  
  // Comparison operations
  equals(other) {
    if (!this._initialized || !other._initialized) return false;
    
    return this._gmp.mpz_cmp(this._mpz, other._mpz) === 0;
  }
  
  lessThan(other) {
    if (!this._initialized || !other._initialized) return false;
    
    return this._gmp.mpz_cmp(this._mpz, other._mpz) < 0;
  }
  
  greaterThan(other) {
    if (!this._initialized || !other._initialized) return false;
    
    return this._gmp.mpz_cmp(this._mpz, other._mpz) > 0;
  }
  
  // Clean up resources
  destroy() {
    if (this._initialized) {
      this._gmp.mpz_clear(this._mpz);
      this._initialized = false;
    }
  }
}

// Create a factory function to create BigInt instances
export async function createBigInt(value = 0, gmpModule) {
  const bigInt = new BigInt(value);
  await bigInt.initialize(gmpModule);
  return bigInt;
}
EOF

# Create a test HTML file for the GMP WebAssembly module
echo "Creating test HTML file for GMP WebAssembly module..."
cat > "${WASM_MODULES_DIR}/gmp_test.html" << 'EOF'
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>GMP WebAssembly Test</title>
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
  <h1>GMP WebAssembly Test</h1>
  
  <button id="testGMP">Test GMP</button>
  <button id="testBigInt">Test BigInt</button>
  <button id="clearOutput">Clear Output</button>
  
  <pre id="output"></pre>

  <script type="module">
    import { initGMPModule, createBigInt } from './gmp/gmp_wrapper.js';
    
    const outputElement = document.getElementById('output');
    
    function log(message) {
      outputElement.textContent += message + '\n';
    }
    
    document.getElementById('clearOutput').addEventListener('click', () => {
      outputElement.textContent = '';
    });
    
    document.getElementById('testGMP').addEventListener('click', async () => {
      try {
        log('Initializing GMP module...');
        const gmp = await initGMPModule();
        log('GMP module initialized successfully!');
        
        log('\nTesting basic GMP functions:');
        
        // Create and initialize mpz_t variables
        const a = new gmp.mpz_t();
        const b = new gmp.mpz_t();
        const result = new gmp.mpz_t();
        
        gmp.mpz_init(a);
        gmp.mpz_init(b);
        gmp.mpz_init(result);
        
        // Set values
        gmp.mpz_set_str(a, "12345678901234567890", 10);
        gmp.mpz_set_str(b, "98765432109876543210", 10);
        
        // Addition
        gmp.mpz_add(result, a, b);
        const addStr = gmp.mpz_get_str(null, 10, result);
        log(`Addition: ${gmp.UTF8ToString(addStr)}`);
        
        // Subtraction
        gmp.mpz_sub(result, b, a);
        const subStr = gmp.mpz_get_str(null, 10, result);
        log(`Subtraction: ${gmp.UTF8ToString(subStr)}`);
        
        // Multiplication
        gmp.mpz_mul(result, a, b);
        const mulStr = gmp.mpz_get_str(null, 10, result);
        log(`Multiplication: ${gmp.UTF8ToString(mulStr)}`);
        
        // Division
        gmp.mpz_fdiv_q(result, b, a);
        const divStr = gmp.mpz_get_str(null, 10, result);
        log(`Division: ${gmp.UTF8ToString(divStr)}`);
        
        // Clean up
        gmp.mpz_clear(a);
        gmp.mpz_clear(b);
        gmp.mpz_clear(result);
        
        log('\nGMP test completed successfully!');
      } catch (error) {
        log(`Error: ${error.message}`);
        console.error(error);
      }
    });
    
    document.getElementById('testBigInt').addEventListener('click', async () => {
      try {
        log('Initializing GMP module for BigInt test...');
        const gmp = await initGMPModule();
        log('GMP module initialized successfully!');
        
        log('\nTesting BigInt wrapper:');
        
        // Create BigInt instances
        const a = await createBigInt("12345678901234567890", gmp);
        const b = await createBigInt("98765432109876543210", gmp);
        
        log(`a = ${a.toString()}`);
        log(`b = ${b.toString()}`);
        
        // Addition
        const sum = await a.add(b);
        log(`a + b = ${sum.toString()}`);
        
        // Subtraction
        const diff = await b.subtract(a);
        log(`b - a = ${diff.toString()}`);
        
        // Multiplication
        const product = await a.multiply(b);
        log(`a * b = ${product.toString()}`);
        
        // Division
        const quotient = await b.divide(a);
        log(`b / a = ${quotient.toString()}`);
        
        // Comparison
        log(`a == b: ${a.equals(b)}`);
        log(`a < b: ${a.lessThan(b)}`);
        log(`a > b: ${a.greaterThan(b)}`);
        
        // Clean up
        a.destroy();
        b.destroy();
        sum.destroy();
        diff.destroy();
        product.destroy();
        quotient.destroy();
        
        log('\nBigInt test completed successfully!');
      } catch (error) {
        log(`Error: ${error.message}`);
        console.error(error);
      }
    });
    
    log('GMP WebAssembly test page loaded. Click the buttons to run tests.');
  </script>
</body>
</html>
EOF

echo "Build process fixed!"
echo "Now run a web server to test the GMP WebAssembly module:"
echo "  cd ${WASM_MODULES_DIR} && python3 -m http.server 8000"
echo "Then open http://localhost:8000/gmp_test.html in your browser"
