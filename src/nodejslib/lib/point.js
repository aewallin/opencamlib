"use strict";
var __importDefault = (this && this.__importDefault) || function (mod) {
    return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", { value: true });
var ocl_1 = __importDefault(require("./ocl"));
var Point = /** @class */ (function () {
    function Point(x, y, z) {
        this.actualClass = new ocl_1.default.Point(x, y, z);
    }
    return Point;
}());
exports.default = Point;
