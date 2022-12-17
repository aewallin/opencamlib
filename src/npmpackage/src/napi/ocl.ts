var buildType = 'Release'
if (process.env.DEBUG) {
    buildType = 'Debug'
}

const platform = process.platform === 'darwin' ? 'macos' : (process.platform === 'win32' ? 'windows' : 'linux')

const oclLib = require(__dirname + '/../../build/' + buildType + '/' + platform + '-nodejs-' + process.arch + '/ocl.node')

export default oclLib