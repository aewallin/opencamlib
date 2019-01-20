var stl = require('stl')
var fs = require('fs');
var facets = stl.toObject(fs.readFileSync('./stl/gnu_tux_mod.stl'));

const Module = require(__dirname + '/../../buildemscripten/src/opencamlib.js')
Module.onRuntimeInitialized = function() {
  const surface = new Module.STLSurf()
  // new Module.STLReader('/mnt/gnu_tux_mod.stl', surface)

  facets.facets.forEach(facet => {
    const tri = new Module.Triangle(
      new Module.Point(facet.verts[0][0], facet.verts[0][1], facet.verts[0][2]),
      new Module.Point(facet.verts[1][0], facet.verts[1][1], facet.verts[1][2]),
      new Module.Point(facet.verts[2][0], facet.verts[2][1], facet.verts[2][2])
    )
    surface.addTriangle(tri)
  })

  console.log(surface.size())
  const cutter = new Module.CylCutter(4, 20)
  const wl = new Module.Waterline()
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
