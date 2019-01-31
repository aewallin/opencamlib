"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
var tslib_1 = require("tslib");
var millingcutter_1 = tslib_1.__importDefault(require("./millingcutter"));
var ConeCutter = /** @class */ (function (_super) {
    tslib_1.__extends(ConeCutter, _super);
    // create a ConeCutter with specified maximum diameter and cone-angle
    // for a 90-degree cone specify the half-angle  angle= pi/4
    function ConeCutter(diameter, angle, length) {
        if (length === void 0) { length = 10; }
        var _this = _super.call(this) || this;
        _this.diameter = diameter;
        _this.angle = angle;
        _this.length = length;
        return _this;
    }
    ConeCutter.prototype.serialize = function () {
        return ['ConeCutter', [this.diameter, this.angle, this.length]];
    };
    return ConeCutter;
}(millingcutter_1.default));
exports.default = ConeCutter;
