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
var Line = /** @class */ (function (_super) {
    __extends(Line, _super);
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
