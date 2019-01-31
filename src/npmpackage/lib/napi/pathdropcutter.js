"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
var tslib_1 = require("tslib");
var operation_1 = tslib_1.__importDefault(require("./operation"));
var ocl_1 = tslib_1.__importDefault(require("./ocl"));
var NapiPathDropCutter = /** @class */ (function (_super) {
    tslib_1.__extends(NapiPathDropCutter, _super);
    function NapiPathDropCutter() {
        var _this = _super.call(this) || this;
        _this.actualClass = new ocl_1.default.PathDropCutter();
        return _this;
    }
    NapiPathDropCutter.prototype.setPath = function (path) {
        var actualPath = new ocl_1.default.Path();
        path.serialize().forEach(function (span) {
            if (span[0] === 'Line') {
                var p1 = new ocl_1.default.Point(span[1][0].x, span[1][0].y, span[1][0].z);
                var p2 = new ocl_1.default.Point(span[1][1].x, span[1][1].y, span[1][1].z);
                var l = new ocl_1.default.Line(p1, p2);
                actualPath.append(l);
            }
        });
        this.actualClass.setPath(actualPath);
    };
    NapiPathDropCutter.prototype.run = function () {
        this.actualClass.run();
        return Promise.resolve(this.actualClass.getCLPoints());
    };
    return NapiPathDropCutter;
}(operation_1.default));
exports.default = NapiPathDropCutter;
