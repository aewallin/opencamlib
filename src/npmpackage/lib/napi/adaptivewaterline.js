"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
var tslib_1 = require("tslib");
var operation_1 = tslib_1.__importDefault(require("./operation"));
var ocl_1 = tslib_1.__importDefault(require("./ocl"));
var NodeJSAdaptiveWaterline = /** @class */ (function (_super) {
    tslib_1.__extends(NodeJSAdaptiveWaterline, _super);
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
