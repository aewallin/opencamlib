"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
var tslib_1 = require("tslib");
var span_1 = tslib_1.__importDefault(require("./span"));
var Line = /** @class */ (function (_super) {
    tslib_1.__extends(Line, _super);
    function Line(p1, p2) {
        var _this = _super.call(this) || this;
        _this.p1 = p1;
        _this.p2 = p2;
        return _this;
    }
    Line.prototype.serialize = function () {
        return ['Line', [this.p1, this.p2]];
    };
    return Line;
}(span_1.default));
exports.default = Line;
