var buildType = 'Release'
if (process.env.DEBUG) {
    buildType = 'Debug'
}

const oclLib = require(__dirname + '/../../build/' + buildType + '/ocl-' + process.platform + '-' + process.arch + '.node')

export default oclLib