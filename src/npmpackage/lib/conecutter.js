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
var millingcutter_1 = __importDefault(require("./millingcutter"));
var ConeCutter = /** @class */ (function (_super) {
    __extends(ConeCutter, _super);
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
