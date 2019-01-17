"use strict";
var __importDefault = (this && this.__importDefault) || function (mod) {
    return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", { value: true });
var ocl_1 = __importDefault(require("./ocl"));
var STLReader = /** @class */ (function () {
    function STLReader(filepath, surface) {
        this.actualClass = new ocl_1.default.STLReader(filepath, surface.actualClass);
    }
    return STLReader;
}());
exports.default = STLReader;
