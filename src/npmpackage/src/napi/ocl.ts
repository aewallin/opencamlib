var buildType = 'Release'
if (process.env.DEBUG) {
    buildType = 'Debug'
}

const platform = process.platform === 'darwin' ? 'macos' : (process.platform === 'win32' ? 'windows' : 'linux')

const oclPath = process.env.OCL_NODE_PATH || (__dirname + '/../../build/' + buildType + '/' + platform + '-nodejs-' + process.arch + '/ocl.node')
const oclLib = require(oclPath)

export default oclLib
