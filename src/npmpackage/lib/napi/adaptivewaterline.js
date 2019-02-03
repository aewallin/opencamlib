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
var operation_1 = __importDefault(require("./operation"));
var ocl_1 = __importDefault(require("./ocl"));
var NodeJSAdaptiveWaterline = /** @class */ (function (_super) {
    __extends(NodeJSAdaptiveWaterline, _super);
    function NodeJSAdaptiveWaterline() {
        var _this = _super.call(this) || this;
        _this.actualClass = new ocl_1.default.AdaptiveWaterline();
        return _this;
    }
    NodeJSAdaptiveWaterline.prototype.setZ = function (z) {
        this.actualClass.setZ(z);
    };
    NodeJSAdaptiveWaterline.prototype.setMinSampling = function (minSampling) {
        this.actualClass.setMinSampling(minSampling);
    };
    NodeJSAdaptiveWaterline.prototype.run = function () {
        this.actualClass.run();
        return Promise.resolve(this.actualClass.getLoops());
    };
    return NodeJSAdaptiveWaterline;
}(operation_1.default));
exports.default = NodeJSAdaptiveWaterline;
