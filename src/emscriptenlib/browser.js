var stl = require('stl')
var fs = require('fs');

const ocl = require('./opencamlib')()

ocl.onRuntimeInitialized = function() {
  const surface = new ocl.STLSurf()
  var facets = stl.toObject(fs.readFileSync('./stl/gnu_tux_mod.stl', 'utf8'));

  facets.facets.forEach(facet => {
    const tri = new ocl.Triangle(
      new ocl.Point(facet.verts[0][0], facet.verts[0][1], facet.verts[0][2]),
      new ocl.Point(facet.verts[1][0], facet.verts[1][1], facet.verts[1][2]),
      new ocl.Point(facet.verts[2][0], facet.verts[2][1], facet.verts[2][2])
    )
    surface.addTriangle(tri)
  })

  console.log(surface.size())
  const cutter = new ocl.CylCutter(4, 20)
  const wl = new ocl.Waterline()
  wl.setSTL(surface)
  wl.setCutter(cutter)
  wl.setZ(1)
  wl.setSampling(0.1)
  wl.run()
  const loops = wl.getLoops()
  for (var i = 0; i < loops.size(); i++) {
    const loop = loops.get(i)
    for (var j = 0; j < loop.size(); j++) {
      const point = loop.get(j)
      console.log('G01 X' + point.x + ' Y' + point.y + ' Z' + point.z)
    }
  }
}

