"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
var tslib_1 = require("tslib");
var operation_1 = tslib_1.__importDefault(require("./operation"));
var ocl_1 = tslib_1.__importDefault(require("./ocl"));
var EmscriptenWaterline = /** @class */ (function (_super) {
    tslib_1.__extends(EmscriptenWaterline, _super);
    function EmscriptenWaterline() {
        var _this = _super.call(this) || this;
        _this.chain = _this.chain.then(function () {
            _this.actualClass = new ocl_1.default.Waterline();
        });
        return _this;
    }
    EmscriptenWaterline.prototype.setZ = function (z) {
        var _this = this;
        this.chain = this.chain.then(function () {
            _this.actualClass.setZ(z);
        });
    };
    EmscriptenWaterline.prototype.loopsToArray = function (loops) {
        var result = [];
        for (var i = 0; i < loops.size(); i++) {
            var loop = loops.get(i);
            var points = [];
            for (var j = 0; j < loop.size(); j++) {
                var point = loop.get(j);
                points.push([point.x, point.y, point.z]);
            }
            result.push(points);
        }
        return result;
    };
    EmscriptenWaterline.prototype.run = function () {
        var _this = this;
        return new Promise(function (resolve) {
            _this.chain.then(function () {
                _this.actualClass.run();
                resolve(_this.loopsToArray(_this.actualClass.getLoops()));
            });
        });
    };
    return EmscriptenWaterline;
}(operation_1.default));
exports.default = EmscriptenWaterline;
