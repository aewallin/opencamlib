"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
var tslib_1 = require("tslib");
var millingcutter_1 = tslib_1.__importDefault(require("./millingcutter"));
var CylCutter = /** @class */ (function (_super) {
    tslib_1.__extends(CylCutter, _super);
    function CylCutter(diameter, length) {
        var _this = _super.call(this) || this;
        _this.diameter = diameter;
        _this.length = length;
        return _this;
    }
    CylCutter.prototype.serialize = function () {
        return ['CylCutter', [this.diameter, this.length]];
    };
    return CylCutter;
}(millingcutter_1.default));
exports.default = CylCutter;
