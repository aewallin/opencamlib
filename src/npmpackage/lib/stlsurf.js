"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
var STLSurf = /** @class */ (function () {
    function STLSurf() {
    }
    STLSurf.prototype.serialize = function () {
        return this.triangles;
    };
    return STLSurf;
}());
exports.default = STLSurf;
