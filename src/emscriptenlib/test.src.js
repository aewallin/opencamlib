const stl = require('stl')
const ocl = require('./index')
const fetch = require('isomorphic-fetch')

function waterline(surface, cutter, z, sampling) {
    console.log('Waterline')
    const wl = new ocl.Waterline()
    wl.setSTL(surface)
    wl.setCutter(cutter)
    wl.setZ(z)
    wl.setSampling(sampling)
    wl.run()
    let gcode = ''
    const loops = wl.getLoops()
    for (var i = 0; i < loops.size(); i++) {
        const loop = loops.get(i)
        for (var j = 0; j < loop.size(); j++) {
            const point = loop.get(j)
            gcode += 'G01 X' + point.x + ' Y' + point.y + ' Z' + point.z + '\n'
        }
    }
    console.log(gcode)
}

function adaptiveWaterline(surface, cutter, z, sampling, minSampling) {
    console.log('AdaptiveWaterline')
    const awl = new ocl.AdaptiveWaterline()
    awl.setSTL(surface)
    awl.setCutter(cutter)
    awl.setZ(z)
    awl.setSampling(sampling)
    awl.setMinSampling(minSampling)
    awl.run()
    let gcode = ''
    const loops = awl.getLoops()
    for (var i = 0; i < loops.size(); i++) {
        const loop = loops.get(i)
        for (var j = 0; j < loop.size(); j++) {
            const point = loop.get(j)
            gcode += 'G01 X' + point.x + ' Y' + point.y + ' Z' + point.z + '\n'
        }
    }
    console.log(gcode)
}

function pathDropCutter(surface, cutter, sampling, path) {
    console.log('PathDropCutter')
    const pdc = new ocl.PathDropCutter()
    pdc.setSTL(surface)
    pdc.setCutter(cutter)
    pdc.setPath(path)
    pdc.setSampling(sampling)
    pdc.run()
    const points = pdc.getPoints()
    let gcode = ''
    for (var j = 0; j < points.size(); j++) {
        const point = points.get(j)
        gcode += 'G01 X' + point.x + ' Y' + point.y + ' Z' + point.z + '\n'
    }
    console.log(gcode)
}

function adaptivePathDropCutter(surface, cutter, sampling, minSampling, path) {
    console.log('AdaptivePathDropCutter')
    const apdc = new ocl.AdaptivePathDropCutter()
    apdc.setSTL(surface)
    apdc.setCutter(cutter)
    apdc.setPath(path)
    apdc.setSampling(sampling)
    apdc.setMinSampling(minSampling)
    apdc.run()
    const points = apdc.getPoints()
    let gcode = ''
    for (var j = 0; j < points.size(); j++) {
        const point = points.get(j)
        gcode += 'G01 X' + point.x + ' Y' + point.y + ' Z' + point.z + '\n'
    }
    console.log(gcode)
}

ocl.then(function () {
    fetch('https://cdn.jsdelivr.net/gh/aewallin/opencamlib@master/stl/gnu_tux_mod.stl')
        .then(res => res.text())
        .then(stlContents => {
            var facets = stl.toObject(stlContents)
            const surface = new ocl.STLSurf()
            facets.facets.forEach(facet => {
                const tri = new ocl.Triangle(
                    new ocl.Point(facet.verts[0][0], facet.verts[0][1], facet.verts[0][2]),
                    new ocl.Point(facet.verts[1][0], facet.verts[1][1], facet.verts[1][2]),
                    new ocl.Point(facet.verts[2][0], facet.verts[2][1], facet.verts[2][2])
                )
                surface.addTriangle(tri)
            })
            const cutter = new ocl.CylCutter(4, 20)
            waterline(surface, cutter, 1, 0.1)
            adaptiveWaterline(surface, cutter, 1, 0.1, 0.001)
            const path = new ocl.Path()
            const p1 = new ocl.Point(0, 1, 0)
            const p2 = new ocl.Point(10, 1, 0)
            const l = new ocl.Line(p1, p2)
            path.appendLine(l)
            pathDropCutter(surface, cutter, 0.1, path)
            adaptivePathDropCutter(surface, cutter, 0.04, 0.01, path)
        })
})