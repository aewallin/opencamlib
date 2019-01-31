"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
var tslib_1 = require("tslib");
var operation_1 = tslib_1.__importDefault(require("./operation"));
var ocl_1 = tslib_1.__importDefault(require("./ocl"));
var EmscriptenAdaptivePathDropCutter = /** @class */ (function (_super) {
    tslib_1.__extends(EmscriptenAdaptivePathDropCutter, _super);
    function EmscriptenAdaptivePathDropCutter() {
        var _this = _super.call(this) || this;
        _this.chain = _this.chain.then(function () {
            _this.actualClass = new ocl_1.default.AdaptivePathDropCutter();
        });
        return _this;
    }
    EmscriptenAdaptivePathDropCutter.prototype.setMinSampling = function (minSampling) {
        var _this = this;
        this.chain = this.chain.then(function () {
            _this.actualClass.setMinSampling(minSampling);
        });
    };
    EmscriptenAdaptivePathDropCutter.prototype.setPath = function (path) {
        var _this = this;
        this.chain = this.chain.then(function () {
            var actualPath = new ocl_1.default.Path();
            path.serialize().forEach(function (span) {
                if (span[0] === 'Line') {
                    var p1 = new ocl_1.default.Point(span[1][0].x, span[1][0].y, span[1][0].z);
                    var p2 = new ocl_1.default.Point(span[1][1].x, span[1][1].y, span[1][1].z);
                    var l = new ocl_1.default.Line(p1, p2);
                    actualPath.append(l);
                }
            });
            _this.actualClass.setPath(actualPath);
        });
    };
    EmscriptenAdaptivePathDropCutter.prototype.run = function () {
        var _this = this;
        return this.chain.then(function () {
            _this.actualClass.run();
            return Promise.resolve(_this.actualClass.getCLPoints());
        });
    };
    return EmscriptenAdaptivePathDropCutter;
}(operation_1.default));
exports.default = EmscriptenAdaptivePathDropCutter;
