var platform = 'linux'
if (process.platform == 'win32') {
    platform = 'win32'
} else if (process.platform == 'darwin') {
    platform = 'darwin'
}
var buildType = 'Release'
if (process.env.DEBUG) {
    buildType = 'Debug'
}

const oclLib = require(__dirname + '/../../../build/' + buildType + '/opencamlib.' + platform + '.node')

export default oclLib