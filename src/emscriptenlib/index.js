if (typeof window !== 'undefined') {
    window.ocl = require('./opencamlib')()
}

module.exports = require('./opencamlib')()