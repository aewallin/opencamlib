"use strict";
var __extends = (this && this.__extends) || (function () {
    var extendStatics = function (d, b) {
        extendStatics = Object.setPrototypeOf ||
            ({ __proto__: [] } instanceof Array && function (d, b) { d.__proto__ = b; }) ||
            function (d, b) { for (var p in b) if (Object.prototype.hasOwnProperty.call(b, p)) d[p] = b[p]; };
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
var AdaptiveWaterline = /** @class */ (function (_super) {
    __extends(AdaptiveWaterline, _super);
    function AdaptiveWaterline() {
        var _this = _super.call(this) || this;
        _this.chain = _this.chain.then(function () {
            _this.actualClass = new ocl_1.default.AdaptiveWaterline();
        });
        return _this;
    }
    AdaptiveWaterline.prototype.setMinSampling = function (minSampling) {
        var _this = this;
        this.chain = this.chain.then(function () {
            _this.actualClass.setMinSampling(minSampling);
        });
    };
    AdaptiveWaterline.prototype.setZ = function (z) {
        var _this = this;
        this.chain = this.chain.then(function () {
            _this.actualClass.setZ(z);
        });
    };
    AdaptiveWaterline.prototype.loopsToArray = function (loops) {
        var result = [];
        for (var i = 0; i < loops.size(); i++) {
            var loop = loops.get(i);
            var points = [];
            for (var j = 0; j < loop.size(); j++) {
                var point = loop.get(j);
                points.push([point.x, point.y, point.z]);
            }
            result.push(points);
        }
        return result;
    };
    AdaptiveWaterline.prototype.run = function () {
        var _this = this;
        return new Promise(function (resolve) {
            _this.chain.then(function () {
                _this.actualClass.run();
                resolve(_this.loopsToArray(_this.actualClass.getLoops()));
            });
        });
    };
    return AdaptiveWaterline;
}(operation_1.default));
exports.default = AdaptiveWaterline;
