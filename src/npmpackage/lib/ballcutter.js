"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
var tslib_1 = require("tslib");
var millingcutter_1 = tslib_1.__importDefault(require("./millingcutter"));
var BallCutter = /** @class */ (function (_super) {
    tslib_1.__extends(BallCutter, _super);
    function BallCutter(diameter, length) {
        var _this = _super.call(this) || this;
        _this.diameter = diameter;
        _this.length = length;
        return _this;
    }
    BallCutter.prototype.serialize = function () {
        return ['BallCutter', [this.diameter, this.length]];
    };
    return BallCutter;
}(millingcutter_1.default));
exports.default = BallCutter;
