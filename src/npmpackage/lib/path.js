"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
var Path = /** @class */ (function () {
    function Path() {
        this.segments = [];
    }
    Path.prototype.append = function (line) {
        this.segments.push(line);
    };
    Path.prototype.serialize = function () {
        return this.segments.map(function (segment) { return segment.serialize(); });
    };
    return Path;
}());
exports.default = Path;
