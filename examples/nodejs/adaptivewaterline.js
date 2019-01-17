const ocl = require('../..')
const {
    STLSurf,
    STLReader,
    CylCutter,
    AdaptiveWaterline
} = ocl

const surface = new STLSurf()
new STLReader(__dirname + '/../../stl/gnu_tux_mod.stl', surface)
const cutter = new CylCutter(4, 20)
const awl = new AdaptiveWaterline()
awl.setSTL(surface)
awl.setCutter(cutter)
awl.setSampling(0.1)
awl.setMinSampling(0.001)
awl.setZ(1)
awl.run()
awl.getLoops().forEach(function (loop) {
    loop.forEach(function (point) {
        console.log('G01 X' + point[0] + ' Y' + point[1] + ' Z' + point[2])
    })
})