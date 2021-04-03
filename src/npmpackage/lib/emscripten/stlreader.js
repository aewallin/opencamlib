"use strict";
var __importDefault = (this && this.__importDefault) || function (mod) {
    return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", { value: true });
var stl_1 = __importDefault(require("stl"));
var STLReader = /** @class */ (function () {
    function STLReader(contents, surface) {
        var obj = stl_1.default.toObject(contents);
        surface.triangles = obj.facets.map(function (facet) { return facet.verts; });
    }
    return STLReader;
}());
exports.default = STLReader;
