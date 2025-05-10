// LMVS JavaScript wrapper
import { initGMPModule, createBigInt } from '../gmp/gmp_wrapper.js';
import lmvsInit from './lmvs.js';

// Initialize the LMVS module
export async function initLMVSModule() {
  try {
    // First initialize the GMP module
    const gmp = await initGMPModule();
    console.log("GMP module initialized:", gmp);

    // Then initialize the LMVS module
    console.log("LMVS init function:", lmvsInit);
    const lmvs = lmvsInit();
    console.log("LMVS module initialized:", lmvs);

    return {
      lmvs,
      gmp
    };
  } catch (error) {
    console.error("Error initializing modules:", error);
    throw error;
  }
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
