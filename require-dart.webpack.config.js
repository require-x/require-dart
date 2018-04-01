module.exports = {
  module: {
    rules: [
      {
        test: /\.js$/,
        exclude: /(node_modules|bower_components)/,
        loader: 'babel-loader',
        query: {
          presets: ['es2015']
        }
      }
    ]
  },
  node: {
    console: true,
    global: false,
    process: true,
    __filename: false,
    __dirname: false,
    Buffer: true,
    setImmediate: true,
  },
};