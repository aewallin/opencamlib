const BENCH = false

BENCH && console.time('load')
const ocl = require('opencamlib')

const fs = require('fs')
const stl = require('stl')

BENCH && console.timeEnd('load')
const {
    STLSurf,
    STLReader,
    CylCutter,
    Waterline,
} = ocl

BENCH && console.time('loadstl')
const surface = new STLSurf()
new STLReader(fs.readFileSync(__dirname + '/../../stl/gnu_tux_mod.stl'), surface)
BENCH && console.timeEnd('loadstl')

const cutter = new CylCutter(4, 20)
const wl = new Waterline()
wl.setSTL(surface)
wl.setCutter(cutter)
wl.setZ(1)
wl.setSampling(0.1)
BENCH && console.time('run')
wl.run().then(loops => {
    BENCH && console.timeEnd('run')
    loops.forEach((loop) => {
        BENCH && console.timeEnd('loops')
        loop.forEach((point) => {
            console.log('G01 X' + point[0] + ' Y' + point[1] + ' Z' + point[2])
        })
    })
})