"use strict";
var __importDefault = (this && this.__importDefault) || function (mod) {
    return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", { value: true });
var ocl_1 = __importDefault(require("./ocl"));
var Line = /** @class */ (function () {
    function Line(p1, p2) {
        this.actualClass = new ocl_1.default.Line(p1.actualClass, p2.actualClass);
    }
    return Line;
}());
exports.default = Line;
