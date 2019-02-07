const ocl = require('../../src/npmpackage')
const fs = require('fs')
const {
    STLSurf,
    STLReader,
    CylCutter,
    AdaptiveWaterline
} = ocl

const surface = new STLSurf()
new STLReader(fs.readFileSync(__dirname + '/../../stl/gnu_tux_mod.stl', 'utf8'), surface)
const cutter = new CylCutter(1, 20)
for (var z = 0; z < 3; z++) {
    const awl = new AdaptiveWaterline()
    awl.setSTL(surface)
    awl.setCutter(cutter)
    awl.setSampling(0.1)
    awl.setMinSampling(0.01)
    awl.setZ(z)
    awl.run().then(loops => {
        loops.forEach(points => {
            points.forEach(function (point) {
                console.log('G01 X' + point[0] + ' Y' + point[1] + ' Z' + point[2])
            })
        })
    })
}