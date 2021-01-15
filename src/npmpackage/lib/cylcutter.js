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
var CylCutter = /** @class */ (function (_super) {
    __extends(CylCutter, _super);
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
