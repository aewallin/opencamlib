const fs = require('fs')
const {
    STLSurf,
    STLReader,
    CylCutter,
    Waterline,
    Point,
    Triangle
} = require('opencamlib/lib/emscripten')

async function main() {
    const file = fs.readFileSync(__dirname + '/../../stl/gnu_tux_mod.stl')
    const surface = new STLSurf()
    console.time('loadstl')
    new STLReader(file, surface)
    console.timeEnd('loadstl')
    const cutter = new CylCutter(4, 20)
    const wl = new Waterline()
    wl.setSTL(surface)
    wl.setCutter(cutter)
    wl.setZ(1)
    wl.setSampling(0.1)
    console.time('run')
    const loops = await wl.run()
    console.timeEnd('run')
    loops.forEach(function (loop) {
        loop.forEach(function (point) {
            console.log('G01 X' + point[0] + ' Y' + point[1] + ' Z' + point[2])
        })
    })
}

main()