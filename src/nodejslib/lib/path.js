"use strict";
var __importDefault = (this && this.__importDefault) || function (mod) {
    return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", { value: true });
var ocl_1 = __importDefault(require("./ocl"));
var Path = /** @class */ (function () {
    function Path() {
        this.actualClass = new ocl_1.default.Path();
    }
    Path.prototype.append = function (line) {
        this.actualClass.append(line.actualClass);
    };
    return Path;
}());
exports.default = Path;
