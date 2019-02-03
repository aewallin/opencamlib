"use strict";
var __importDefault = (this && this.__importDefault) || function (mod) {
    return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", { value: true });
var ocl_1 = __importDefault(require("./ocl"));
var cylcutter_1 = __importDefault(require("../cylcutter"));
var ballcutter_1 = __importDefault(require("../ballcutter"));
var bullcutter_1 = __importDefault(require("../bullcutter"));
var conecutter_1 = __importDefault(require("../conecutter"));
var Operation = /** @class */ (function () {
    function Operation() {
    }
    Operation.prototype.setSTL = function (surface) {
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
        this.actualClass.setSTL(actualSurface);
    };
    Operation.prototype.setCutter = function (cutter) {
        var cutterDef = cutter.serialize();
        var cutterType = cutterDef[0], cutterArgs = cutterDef[1];
        var Cutter = ocl_1.default[cutterType];
        var actualCutter = new (Cutter.bind.apply(Cutter, [void 0].concat(cutterArgs)))();
        if (cutter instanceof cylcutter_1.default) {
            this.actualClass.setCylCutter(actualCutter);
        }
        else if (cutter instanceof ballcutter_1.default) {
            this.actualClass.setBallCutter(actualCutter);
        }
        else if (cutter instanceof bullcutter_1.default) {
            this.actualClass.setBullCutter(actualCutter);
        }
        else if (cutter instanceof conecutter_1.default) {
            this.actualClass.setConeCutter(actualCutter);
        }
    };
    Operation.prototype.setSampling = function (sampling) {
        this.actualClass.setSampling(sampling);
    };
    return Operation;
}());
exports.default = Operation;
