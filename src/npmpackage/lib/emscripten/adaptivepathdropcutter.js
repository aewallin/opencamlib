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
var operation_1 = __importDefault(require("./operation"));
var ocl_1 = __importDefault(require("./ocl"));
var EmscriptenAdaptivePathDropCutter = /** @class */ (function (_super) {
    __extends(EmscriptenAdaptivePathDropCutter, _super);
    function EmscriptenAdaptivePathDropCutter() {
        var _this = _super.call(this) || this;
        _this.chain = _this.chain.then(function () {
            _this.actualClass = new ocl_1.default.AdaptivePathDropCutter();
        });
        return _this;
    }
    EmscriptenAdaptivePathDropCutter.prototype.setMinSampling = function (minSampling) {
        var _this = this;
        this.chain = this.chain.then(function () {
            _this.actualClass.setMinSampling(minSampling);
        });
    };
    EmscriptenAdaptivePathDropCutter.prototype.setPath = function (path) {
        var _this = this;
        this.chain = this.chain.then(function () {
            var actualPath = new ocl_1.default.Path();
            path.serialize().forEach(function (span) {
                if (span[0] === 'Line') {
                    var p1 = new ocl_1.default.Point(span[1][0].x, span[1][0].y, span[1][0].z);
                    var p2 = new ocl_1.default.Point(span[1][1].x, span[1][1].y, span[1][1].z);
                    var l = new ocl_1.default.Line(p1, p2);
                    actualPath.appendLine(l);
                }
            });
            _this.actualClass.setPath(actualPath);
        });
    };
    EmscriptenAdaptivePathDropCutter.prototype.run = function () {
        var _this = this;
        return this.chain.then(function () {
            _this.actualClass.run();
            return Promise.resolve(_this.pointsToArray(_this.actualClass.getPoints()));
        });
    };
    return EmscriptenAdaptivePathDropCutter;
}(operation_1.default));
exports.default = EmscriptenAdaptivePathDropCutter;
