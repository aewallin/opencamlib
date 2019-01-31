"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
var tslib_1 = require("tslib");
var stl_1 = tslib_1.__importDefault(require("stl"));
var STLReader = /** @class */ (function () {
    function STLReader(contents, surface) {
        var obj = stl_1.default.toObject(contents);
        surface.triangles = obj.facets.map(function (facet) { return facet.verts; });
    }
    return STLReader;
}());
exports.default = STLReader;
