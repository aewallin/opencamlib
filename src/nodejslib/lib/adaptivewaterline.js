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
var AdaptiveWaterline = /** @class */ (function (_super) {
    __extends(AdaptiveWaterline, _super);
    function AdaptiveWaterline() {
        return _super !== null && _super.apply(this, arguments) || this;
    }
    AdaptiveWaterline.prototype.setZ = function (z) {
        this.z = z;
    };
    AdaptiveWaterline.prototype.setMinSampling = function (minSampling) {
        this.minSampling = minSampling;
    };
    AdaptiveWaterline.prototype.getLoops = function () {
        if (!this.actualClass) {
            throw new Error('Call run() before getLoops()');
        }
        return this.actualClass.getLoops();
    };
    AdaptiveWaterline.prototype.run = function () {
        this.actualClass = new ocl_1.default.AdaptiveWaterline();
        if (!this.surface) {
            throw new Error('Please set a STLSurface using setSTL()');
        }
        if (!this.cutter) {
            throw new Error('Please set a MillingCutter using setCutter()');
        }
        this.actualClass.setSTL(this.surface.actualClass);
        this.setCutterOnActualClass();
        this.actualClass.setZ(this.z);
        this.actualClass.setSampling(this.sampling);
        this.actualClass.setMinSampling(this.minSampling);
        this.actualClass.run();
    };
    return AdaptiveWaterline;
}(operation_1.default));
exports.default = AdaptiveWaterline;
