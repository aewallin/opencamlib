const ocl = require('../..')
const {
    STLSurf,
    STLReader,
    CylCutter,
    BallCutter,
    BullCutter,
    ConeCutter,
    AdaptivePathDropCutter,
    Path,
    Line,
    Point
} = ocl

function assert(condition, message) {
    if (!condition) {
        throw message || "Assertion failed";
    }
}

// create a simple "Zig" pattern where we cut only in one direction.
// the first line is at ymin
// the last line is at ymax
function YdirectionZigPath(xmin, xmax, ymin, ymax, Ny) {
    const paths = []
    const dy = (ymax - ymin) / (Ny - 1) // the y step-over
    for (let n = 0; n < Ny; n++) {
        const path = new Path() 
        const y = ymin + n * dy // current y-coordinate 
        if (n === Ny - 1) {
            assert(y == ymax)
        } else if (n === 0) {
            assert(y == ymin)
        }
        const p1 = new Point(xmin, y, 0) // start-point of line
        const p2 = new Point(xmax, y, 0) // end-point of line
        const l = new Line(p1, p2) // line-object
        path.append(l) // add the line to the path
        paths.push(path)
    }
    return paths
}

// run the actual drop-cutter algorithm
function adaptivePathDropCutter(surface, cutter, paths) {
    const apdc = new AdaptivePathDropCutter()
    apdc.setSTL(surface)
    apdc.setCutter(cutter)
    // maximum sampling or "step-forward" distance
    // should be set so that we don't loose any detail of the STL model
    // i.e. this number should be similar or smaller than the smallest triangle
    apdc.setSampling(0.04)
    // minimum sampling or step-forward distance
    // the algorithm subdivides "steep" portions of the toolpath
    // until we reach this limit.
    apdc.setMinSampling(0.01)
    let cl_paths = []
    paths.forEach(function (path) {
        apdc.setPath(path)
        apdc.run()
        cl_points = apdc.getCLPoints()
        cl_paths.push(cl_points)
    })
    return cl_paths
}

const surface = new STLSurf()
new STLReader(__dirname + '/../../stl/gnu_tux_mod.stl', surface)
// choose a cutter for the operation:
// http://www.anderswallin.net/2011/08/opencamlib-cutter-shapes/
const diameter = 0.25
const length = 5
// const cutter = new BallCutter(diameter, length)
// const cutter = new CylCutter(diameter, length)
// const corner_radius = 0.05
// const cutter = new BullCutter(diameter, corner_radius, length)
const angle = Math.PI / 4
const cutter = new ConeCutter(diameter, angle, length)
// const cutter = cutter.offsetCutter(0.4)

//  toolpath is contained in this simple box
const ymin = 0
const ymax = 12
const xmin = 0
const xmax = 10
const Ny = 40 // number of lines in the y-direction

const paths = YdirectionZigPath(xmin, xmax, ymin, ymax, Ny)

//  now project onto the STL surface
const toolpaths = adaptivePathDropCutter(surface, cutter, paths)
toolpaths.forEach(function (points) {
    console.log('G00 X' + Math.round(points[0][0] * 10000) / 10000 + ' Y' + Math.round(points[0][1] * 10000) / 10000)
    console.log('G01 Z' + Math.round(points[0][2] * 10000) / 10000)
    points.forEach(function (point) {
        console.log('G01 X' + Math.round(point[0] * 10000) / 10000 + ' Y' + Math.round(point[1] * 10000) / 10000 + ' Z' + Math.round(point[2] * 10000) / 10000)
    })
    console.log('G00 Z3')
})
