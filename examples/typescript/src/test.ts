import * as ocl from '../../../src/npmpackage'
import fetch from 'isomorphic-fetch'

function round(number: number) {
    return Math.round(number * 1000) / 1000
}

function waterline(surface: ocl.STLSurf, cutter: ocl.MillingCutter, sampling: number, z: number) {
    const operation = new ocl.Waterline()
    operation.setSTL(surface)
    operation.setCutter(cutter)
    operation.setSampling(sampling)
    operation.setZ(z)
    return operation.run()
}

function adaptiveWaterline(surface: ocl.STLSurf, cutter: ocl.MillingCutter, sampling: number, minSampling: number, z: number) {
    const operation = new ocl.AdaptiveWaterline()
    operation.setSTL(surface)
    operation.setCutter(cutter)
    operation.setSampling(sampling)
    operation.setMinSampling(minSampling)
    operation.setZ(z)
    return operation.run()
}

function pathDropCutter(surface: ocl.STLSurf, cutter: ocl.MillingCutter, sampling: number, path: ocl.Path) {
    const operation = new ocl.PathDropCutter()
    operation.setSTL(surface)
    operation.setCutter(cutter)
    operation.setSampling(sampling)
    operation.setPath(path)
    return operation.run()
}

function adaptivePathDropCutter(surface: ocl.STLSurf, cutter: ocl.MillingCutter, sampling: number, minSampling: number, path: ocl.Path) {
    const operation = new ocl.AdaptivePathDropCutter()
    operation.setSTL(surface)
    operation.setCutter(cutter)
    operation.setSampling(sampling)
    operation.setMinSampling(minSampling)
    operation.setPath(path)
    return operation.run()
}

function writeLoops(loops: any) {
    // let gcode = ''
    // loops.forEach((loop: any) => {
    //     loop.forEach((point: any) => {
    //         gcode += 'G01 X' + round(point[0]) + ' Y' + round(point[1]) + ' Z' + round(point[2]) + '\n'
    //     })
    // })
    // console.log(gcode)
}

function writeLoop(loop: any) {
    // let gcode = ''
    // loop.forEach((point: any) => {
    //     gcode += 'G01 X' + round(point[0]) + ' Y' + round(point[1]) + ' Z' + round(point[2]) + '\n'
    // })
    // console.log(gcode)
}

async function main() {
    console.time('total')
    console.time('loadSTL')
    const surface = new ocl.STLSurf()
    const res = await fetch('https://cdn.jsdelivr.net/gh/aewallin/opencamlib@master/stl/gnu_tux_mod.stl')
    const stlContents = await res.text()
    new ocl.STLReader(stlContents, surface)
    console.timeEnd('loadSTL')
    const cutter = new ocl.CylCutter(1, 20)
    let chain = Promise.resolve()
    for (var z = -1; z <= 3; z += 0.5) {
        ((z) => {
            chain = chain.then(() => {
                console.time('Waterline@Z' + z)
                return waterline(surface, cutter, 0.1, z)
                    .then(writeLoops)
                    .then(() => console.timeEnd('Waterline@Z' + z))
            })
        })(z)
    }
    for (var z = -1; z <= 3; z += 0.5) {
        ((z) => {
            chain = chain.then(() => {
                console.time('AdaptiveWaterline@Z' + z)
                return adaptiveWaterline(surface, cutter, 0.1, 0.01, z)
                    .then(writeLoops)
                    .then(() => console.timeEnd('AdaptiveWaterline@Z' + z))
            })
        })(z)
    }
    for (var y = 0; y <= 13; y += 0.5) {
        ((y) => {
            const path = new ocl.Path()
            const p1 = new ocl.Point(-0.5, y, 0)
            const p2 = new ocl.Point(9.5, y, 0)
            const l = new ocl.Line(p1, p2)
            path.append(l)
            chain = chain.then(() => {
                console.time('PathDropCutter@Y' + y)
                return pathDropCutter(surface, cutter, 0.1, path)
                    .then(writeLoop)
                    .then(() => console.timeEnd('PathDropCutter@Y' + y))
            })
        })(y)
    }
    for (var y = 0; y <= 13; y += 0.5) {
        ((y) => {
            const path = new ocl.Path()
            const p1 = new ocl.Point(-0.5, y, 0)
            const p2 = new ocl.Point(9.5, y, 0)
            const l = new ocl.Line(p1, p2)
            path.append(l)
            chain = chain.then(() => {
                console.time('AdaptivePathDropCutter@Y' + y)
                return adaptivePathDropCutter(surface, cutter, 0.1, 0.01, path)
                    .then(writeLoop)
                    .then(() => console.timeEnd('AdaptivePathDropCutter@Y' + y))
            })
        })(y)
    }
    chain.then(() => {
        console.timeEnd('total')
        console.log('All done!')
    })
    .catch((err: Error) => {
        console.error(err)
    })
}
main()