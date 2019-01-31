"use strict";
var __awaiter = (this && this.__awaiter) || function (thisArg, _arguments, P, generator) {
    return new (P || (P = Promise))(function (resolve, reject) {
        function fulfilled(value) { try { step(generator.next(value)); } catch (e) { reject(e); } }
        function rejected(value) { try { step(generator["throw"](value)); } catch (e) { reject(e); } }
        function step(result) { result.done ? resolve(result.value) : new P(function (resolve) { resolve(result.value); }).then(fulfilled, rejected); }
        step((generator = generator.apply(thisArg, _arguments || [])).next());
    });
};
var __generator = (this && this.__generator) || function (thisArg, body) {
    var _ = { label: 0, sent: function() { if (t[0] & 1) throw t[1]; return t[1]; }, trys: [], ops: [] }, f, y, t, g;
    return g = { next: verb(0), "throw": verb(1), "return": verb(2) }, typeof Symbol === "function" && (g[Symbol.iterator] = function() { return this; }), g;
    function verb(n) { return function (v) { return step([n, v]); }; }
    function step(op) {
        if (f) throw new TypeError("Generator is already executing.");
        while (_) try {
            if (f = 1, y && (t = op[0] & 2 ? y["return"] : op[0] ? y["throw"] || ((t = y["return"]) && t.call(y), 0) : y.next) && !(t = t.call(y, op[1])).done) return t;
            if (y = 0, t) op = [op[0] & 2, t.value];
            switch (op[0]) {
                case 0: case 1: t = op; break;
                case 4: _.label++; return { value: op[1], done: false };
                case 5: _.label++; y = op[1]; op = [0]; continue;
                case 7: op = _.ops.pop(); _.trys.pop(); continue;
                default:
                    if (!(t = _.trys, t = t.length > 0 && t[t.length - 1]) && (op[0] === 6 || op[0] === 2)) { _ = 0; continue; }
                    if (op[0] === 3 && (!t || (op[1] > t[0] && op[1] < t[3]))) { _.label = op[1]; break; }
                    if (op[0] === 6 && _.label < t[1]) { _.label = t[1]; t = op; break; }
                    if (t && _.label < t[2]) { _.label = t[2]; _.ops.push(op); break; }
                    if (t[2]) _.ops.pop();
                    _.trys.pop(); continue;
            }
            op = body.call(thisArg, _);
        } catch (e) { op = [6, e]; y = 0; } finally { f = t = 0; }
        if (op[0] & 5) throw op[1]; return { value: op[0] ? op[1] : void 0, done: true };
    }
};
var __importStar = (this && this.__importStar) || function (mod) {
    if (mod && mod.__esModule) return mod;
    var result = {};
    if (mod != null) for (var k in mod) if (Object.hasOwnProperty.call(mod, k)) result[k] = mod[k];
    result["default"] = mod;
    return result;
};
var __importDefault = (this && this.__importDefault) || function (mod) {
    return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", { value: true });
var ocl = __importStar(require("../../../src/npmpackage"));
var isomorphic_fetch_1 = __importDefault(require("isomorphic-fetch"));
function round(number) {
    return Math.round(number * 1000) / 1000;
}
function waterline(surface, cutter, sampling, z) {
    var operation = new ocl.Waterline();
    operation.setSTL(surface);
    operation.setCutter(cutter);
    operation.setSampling(sampling);
    operation.setZ(z);
    return operation.run();
}
function adaptiveWaterline(surface, cutter, sampling, minSampling, z) {
    var operation = new ocl.AdaptiveWaterline();
    operation.setSTL(surface);
    operation.setCutter(cutter);
    operation.setSampling(sampling);
    operation.setMinSampling(minSampling);
    operation.setZ(z);
    return operation.run();
}
function pathDropCutter(surface, cutter, sampling, path) {
    var operation = new ocl.PathDropCutter();
    operation.setSTL(surface);
    operation.setCutter(cutter);
    operation.setSampling(sampling);
    operation.setPath(path);
    return operation.run();
}
function adaptivePathDropCutter(surface, cutter, sampling, minSampling, path) {
    var operation = new ocl.AdaptivePathDropCutter();
    operation.setSTL(surface);
    operation.setCutter(cutter);
    operation.setSampling(sampling);
    operation.setMinSampling(minSampling);
    operation.setPath(path);
    return operation.run();
}
function writeLoops(loops) {
    // let gcode = ''
    // loops.forEach((loop: any) => {
    //     loop.forEach((point: any) => {
    //         gcode += 'G01 X' + round(point[0]) + ' Y' + round(point[1]) + ' Z' + round(point[2]) + '\n'
    //     })
    // })
    // console.log(gcode)
}
function writeLoop(loop) {
    // let gcode = ''
    // loop.forEach((point: any) => {
    //     gcode += 'G01 X' + round(point[0]) + ' Y' + round(point[1]) + ' Z' + round(point[2]) + '\n'
    // })
    // console.log(gcode)
}
function main() {
    return __awaiter(this, void 0, void 0, function () {
        var surface, res, stlContents, cutter, chain, z, z, y, y;
        return __generator(this, function (_a) {
            switch (_a.label) {
                case 0:
                    console.time('total');
                    console.time('loadSTL');
                    surface = new ocl.STLSurf();
                    return [4 /*yield*/, isomorphic_fetch_1.default('https://cdn.jsdelivr.net/gh/aewallin/opencamlib@master/stl/gnu_tux_mod.stl')];
                case 1:
                    res = _a.sent();
                    return [4 /*yield*/, res.text()];
                case 2:
                    stlContents = _a.sent();
                    new ocl.STLReader(stlContents, surface);
                    console.timeEnd('loadSTL');
                    cutter = new ocl.CylCutter(1, 20);
                    chain = Promise.resolve();
                    for (z = -1; z <= 3; z += 0.5) {
                        (function (z) {
                            chain = chain.then(function () {
                                console.time('Waterline@Z' + z);
                                return waterline(surface, cutter, 0.1, z)
                                    .then(writeLoops)
                                    .then(function () { return console.timeEnd('Waterline@Z' + z); });
                            });
                        })(z);
                    }
                    for (z = -1; z <= 3; z += 0.5) {
                        (function (z) {
                            chain = chain.then(function () {
                                console.time('AdaptiveWaterline@Z' + z);
                                return adaptiveWaterline(surface, cutter, 0.1, 0.01, z)
                                    .then(writeLoops)
                                    .then(function () { return console.timeEnd('AdaptiveWaterline@Z' + z); });
                            });
                        })(z);
                    }
                    for (y = 0; y <= 13; y += 0.5) {
                        (function (y) {
                            var path = new ocl.Path();
                            var p1 = new ocl.Point(-0.5, y, 0);
                            var p2 = new ocl.Point(9.5, y, 0);
                            var l = new ocl.Line(p1, p2);
                            path.append(l);
                            chain = chain.then(function () {
                                console.time('PathDropCutter@Y' + y);
                                return pathDropCutter(surface, cutter, 0.1, path)
                                    .then(writeLoop)
                                    .then(function () { return console.timeEnd('PathDropCutter@Y' + y); });
                            });
                        })(y);
                    }
                    for (y = 0; y <= 13; y += 0.5) {
                        (function (y) {
                            var path = new ocl.Path();
                            var p1 = new ocl.Point(-0.5, y, 0);
                            var p2 = new ocl.Point(9.5, y, 0);
                            var l = new ocl.Line(p1, p2);
                            path.append(l);
                            chain = chain.then(function () {
                                console.time('AdaptivePathDropCutter@Y' + y);
                                return adaptivePathDropCutter(surface, cutter, 0.1, 0.01, path)
                                    .then(writeLoop)
                                    .then(function () { return console.timeEnd('AdaptivePathDropCutter@Y' + y); });
                            });
                        })(y);
                    }
                    chain.then(function () {
                        console.timeEnd('total');
                        console.log('All done!');
                    })
                        .catch(function (err) {
                        console.error(err);
                    });
                    return [2 /*return*/];
            }
        });
    });
}
main();
