const browserify = require('browserify')
const fs = require('fs')
const path = require('path')
const { pipeline } = require('stream')

// Emscripten emits a self-contained module with modern JS syntax. It must not
// be parsed for CommonJS dependencies or have Node global shims injected.
const bundle = browserify(path.join(__dirname, 'lib/emscripten/index.js'), {
    standalone: 'ocl',
    noParse: [path.join(__dirname, 'build/ocl.js')]
}).plugin(require('browserify-derequire')).bundle()

pipeline(bundle, fs.createWriteStream(path.join(__dirname, 'browser.js')), error => {
    if (error) { console.error(error); process.exitCode = 1 }
})
