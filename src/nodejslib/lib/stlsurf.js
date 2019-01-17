"use strict";
var __importDefault = (this && this.__importDefault) || function (mod) {
    return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", { value: true });
var ocl_1 = __importDefault(require("./ocl"));
var STLSurf = /** @class */ (function () {
    function STLSurf() {
        this.actualClass = new ocl_1.default.STLSurf();
    }
    return STLSurf;
}());
exports.default = STLSurf;
