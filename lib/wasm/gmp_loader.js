// GMP WebAssembly loader
const gmpModule = require('./gmp.js');

// Initialize the GMP module
async function initGMP() {
  const gmp = await gmpModule();
  return gmp;
}

module.exports = {
  initGMP
};
