"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
var tslib_1 = require("tslib");
var operation_1 = tslib_1.__importDefault(require("./operation"));
var ocl_1 = tslib_1.__importDefault(require("./ocl"));
var NodeJSWaterline = /** @class */ (function (_super) {
    tslib_1.__extends(NodeJSWaterline, _super);
    function NodeJSWaterline() {
        var _this = _super.call(this) || this;
        _this.actualClass = new ocl_1.default.Waterline();
        return _this;
    }
    NodeJSWaterline.prototype.setZ = function (z) {
        this.actualClass.setZ(z);
    };
    NodeJSWaterline.prototype.run = function () {
        this.actualClass.run();
        return Promise.resolve(this.actualClass.getLoops());
    };
    return NodeJSWaterline;
}(operation_1.default));
exports.default = NodeJSWaterline;
