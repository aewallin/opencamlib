"use strict";
var __extends = (this && this.__extends) || (function () {
    var extendStatics = function (d, b) {
        extendStatics = Object.setPrototypeOf ||
            ({ __proto__: [] } instanceof Array && function (d, b) { d.__proto__ = b; }) ||
            function (d, b) { for (var p in b) if (b.hasOwnProperty(p)) d[p] = b[p]; };
        return extendStatics(d, b);
    };
    return function (d, b) {
        extendStatics(d, b);
        function __() { this.constructor = d; }
        d.prototype = b === null ? Object.create(b) : (__.prototype = b.prototype, new __());
    };
})();
var __importDefault = (this && this.__importDefault) || function (mod) {
    return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", { value: true });
var span_1 = __importDefault(require("./span"));
var Arc = /** @class */ (function (_super) {
    __extends(Arc, _super);
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
