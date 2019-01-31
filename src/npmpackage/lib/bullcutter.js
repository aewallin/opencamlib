"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
var tslib_1 = require("tslib");
var millingcutter_1 = tslib_1.__importDefault(require("./millingcutter"));
var BullCutter = /** @class */ (function (_super) {
    tslib_1.__extends(BullCutter, _super);
    function BullCutter(diameter, radius, length) {
        var _this = _super.call(this) || this;
        _this.diameter = diameter;
        _this.radius = radius;
        _this.length = length;
        return _this;
    }
    BullCutter.prototype.serialize = function () {
        return ['BullCutter', [this.diameter, this.radius, this.length]];
    };
    return BullCutter;
}(millingcutter_1.default));
exports.default = BullCutter;
