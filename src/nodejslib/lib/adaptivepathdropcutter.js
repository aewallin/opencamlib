"use strict";
var __extends = (this && this.__extends) || (function () {
    var extendStatics = function (d, b) {
        extendStatics = Object.setPrototypeOf ||
            ({ __proto__: [] } instanceof Array && function (d, b) { d.__proto__ = b; }) ||
            function (d, b) { for (var p in b) if (b.hasOwnProperty(p)) d[p] = b[p]; };
        return extendStatics(d, b);
    };
    return function (d, b) {
        extendStatics(d, b);
        function __() { this.constructor = d; }
        d.prototype = b === null ? Object.create(b) : (__.prototype = b.prototype, new __());
    };
})();
var __importDefault = (this && this.__importDefault) || function (mod) {
    return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", { value: true });
var ocl_1 = __importDefault(require("./ocl"));
var operation_1 = __importDefault(require("./operation"));
var AdaptivePathDropCutter = /** @class */ (function (_super) {
    __extends(AdaptivePathDropCutter, _super);
    function AdaptivePathDropCutter() {
        return _super !== null && _super.apply(this, arguments) || this;
    }
    AdaptivePathDropCutter.prototype.setMinSampling = function (minSampling) {
        this.minSampling = minSampling;
    };
    AdaptivePathDropCutter.prototype.getCLPoints = function () {
        if (!this.actualClass) {
            throw new Error('Call run() before getCLPoints()');
        }
        return this.actualClass.getCLPoints();
    };
    AdaptivePathDropCutter.prototype.setPath = function (path) {
        this.path = path;
    };
    AdaptivePathDropCutter.prototype.run = function () {
        this.actualClass = new ocl_1.default.AdaptivePathDropCutter();
        if (!this.surface) {
            throw new Error('Please set a STLSurface using setSTL()');
        }
        if (!this.cutter) {
            throw new Error('Please set a MillingCutter using setCutter()');
        }
        if (!this.path) {
            throw new Error('Please set a Path using setPath()');
        }
        this.actualClass.setSTL(this.surface.actualClass);
        this.setCutterOnActualClass();
        this.actualClass.setPath(this.path.actualClass);
        this.actualClass.setSampling(this.sampling);
        this.actualClass.setMinSampling(this.minSampling);
        this.actualClass.run();
    };
    return AdaptivePathDropCutter;
}(operation_1.default));
exports.default = AdaptivePathDropCutter;
