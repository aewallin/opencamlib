"use strict";
var __importDefault = (this && this.__importDefault) || function (mod) {
    return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", { value: true });
var cylcutter_1 = __importDefault(require("./cylcutter"));
var ballcutter_1 = __importDefault(require("./ballcutter"));
var bullcutter_1 = __importDefault(require("./bullcutter"));
var conecutter_1 = __importDefault(require("./conecutter"));
var Operation = /** @class */ (function () {
    function Operation() {
    }
    Operation.prototype.setSTL = function (surface) {
        this.surface = surface;
    };
    Operation.prototype.setCutter = function (cutter) {
        this.cutter = cutter;
    };
    Operation.prototype.setSampling = function (sampling) {
        this.sampling = sampling;
    };
    Operation.prototype.setCutterOnActualClass = function () {
        if (!this.cutter)
            return;
        if (this.cutter instanceof cylcutter_1.default) {
            this.actualClass.setCylCutter(this.cutter.actualClass);
        }
        else if (this.cutter instanceof ballcutter_1.default) {
            this.actualClass.setBallCutter(this.cutter.actualClass);
        }
        else if (this.cutter instanceof bullcutter_1.default) {
            this.actualClass.setBullCutter(this.cutter.actualClass);
        }
        else if (this.cutter instanceof conecutter_1.default) {
            this.actualClass.setConeCutter(this.cutter.actualClass);
        }
    };
    return Operation;
}());
exports.default = Operation;
