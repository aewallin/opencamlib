"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
function lib(engine) {
    if (engine === void 0) { engine = 'emscripten'; }
    var ocl = require("./" + engine);
    console.log('ocl', ocl);
}
module.exports = lib;
exports.default = lib;
