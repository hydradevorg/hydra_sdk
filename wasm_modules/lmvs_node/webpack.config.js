const path = require('path');

module.exports = {
  entry: './index.js',
  output: {
    filename: 'lmvs.js',
    path: path.resolve(__dirname, '../lmvs'),
    library: {
      type: 'module'
    }
  },
  experiments: {
    outputModule: true
  },
  mode: 'production',
  optimization: {
    minimize: true
  }
};
