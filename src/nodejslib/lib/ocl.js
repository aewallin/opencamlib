"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
var platform = 'linux';
if (process.platform == 'win32') {
    platform = 'win32';
}
else if (process.platform == 'darwin') {
    platform = 'darwin';
}
var buildType = 'Release';
if (process.env.DEBUG) {
    buildType = 'Debug';
}
var oclLib = require(__dirname + '/../../../build/' + buildType + '/opencamlib.' + platform + '.node');
exports.default = oclLib;
