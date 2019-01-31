"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
var tslib_1 = require("tslib");
var span_1 = tslib_1.__importDefault(require("./span"));
var Arc = /** @class */ (function (_super) {
    tslib_1.__extends(Arc, _super);
    // create an arc from point p1 to point p2 with center c and direction dir.
    function Arc(p1, p2, c, antiClockwise) {
        if (antiClockwise === void 0) { antiClockwise = false; }
        var _this = _super.call(this) || this;
        _this.p1 = p1;
        _this.p2 = p2;
        _this.c = c;
        _this.antiClockwise = antiClockwise;
        return _this;
    }
    Arc.prototype.serialize = function () {
        return ['Arc', [this.p1, this.p2, this.c, this.antiClockwise]];
    };
    return Arc;
}(span_1.default));
exports.default = Arc;
