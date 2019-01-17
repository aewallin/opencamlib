const {
    STLSurf,
    STLReader,
    CylCutter,
    Waterline
} = require('../..')

const surface = new STLSurf()
new STLReader(__dirname + '/../../stl/gnu_tux_mod.stl', surface)
const cutter = new CylCutter(4, 20)
const wl = new Waterline()
wl.setSTL(surface)
wl.setCutter(cutter)
wl.setZ(1)
wl.setSampling(0.1)
wl.run()
wl.getLoops().forEach(function (loop) {
    loop.forEach(function (point) {
        console.log('G01 X' + point[0] + ' Y' + point[1] + ' Z' + point[2])
    })
})