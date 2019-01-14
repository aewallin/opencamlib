var platform = 'linux'
if (process.platform == 'win32') {
	platform = 'win32'
} else if (process.platform == 'darwin') {
	platform = 'darwin'
}
module.exports = require(__dirname + '/../../build/Release/OpenCamLib.' + platform + '.node')