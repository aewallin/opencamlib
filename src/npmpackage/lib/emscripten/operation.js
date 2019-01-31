"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
var tslib_1 = require("tslib");
var ocl_1 = tslib_1.__importDefault(require("./ocl"));
var Operation = /** @class */ (function () {
    function Operation() {
        this.chain = new Promise(function (resolve) {
            ocl_1.default.then(function () {
                resolve();
            });
        });
    }
    Operation.prototype.setSTL = function (surface) {
        var _this = this;
        this.chain = this.chain.then(function () {
            if (!surface.triangles) {
                throw new Error('Please provide a valid STLSurf, use STLReader to read a file into it');
            }
            var actualSurface = new ocl_1.default.STLSurf();
            surface.triangles.forEach(function (vertice) {
                var p1 = new ocl_1.default.Point(vertice[0][0], vertice[0][1], vertice[0][2]);
                var p2 = new ocl_1.default.Point(vertice[1][0], vertice[1][1], vertice[1][2]);
                var p3 = new ocl_1.default.Point(vertice[2][0], vertice[2][1], vertice[2][2]);
                var triangle = new ocl_1.default.Triangle(p1, p2, p3);
                actualSurface.addTriangle(triangle);
            });
            _this.actualClass.setSTL(actualSurface);
        });
    };
    Operation.prototype.setCutter = function (cutter) {
        var _this = this;
        this.chain = this.chain.then(function () {
            var cutterDef = cutter.serialize();
            var cutterType = cutterDef[0], cutterArgs = cutterDef[1];
            var Cutter = ocl_1.default[cutterType];
            var actualCutter = new (Cutter.bind.apply(Cutter, [void 0].concat(cutterArgs)))();
            _this.actualClass.setCutter(actualCutter);
        });
    };
    Operation.prototype.setSampling = function (sampling) {
        var _this = this;
        this.chain = this.chain.then(function () {
            _this.actualClass.setSampling(sampling);
        });
    };
    return Operation;
}());
exports.default = Operation;
