// GMP WebAssembly wrapper
import './gmp.js';

// Initialize the GMP module
export async function initGMPModule() {
  // The GMP module is loaded as a global variable called 'Module'
  return new Promise((resolve) => {
    // If Module is already defined, use it
    if (typeof Module !== 'undefined') {
      resolve(Module);
    } else {
      // Otherwise, wait for it to be defined
      window.moduleReady = (module) => {
        resolve(module);
      };
    }
  });
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
