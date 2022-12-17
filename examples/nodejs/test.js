const fs = require('fs')
const {
  Waterline,
  AdaptiveWaterline,
  PathDropCutter,
  AdaptivePathDropCutter,
  STLReader,
  STLSurf,
  CylCutter,
  Path,
  Point,
  Line
} = require('@opencamlib/opencamlib')

function pointToXYZ(point) {
  return `X${Math.round(point[0] * 100000) / 100000} Y${Math.round(point[1] * 100000) / 100000} Z${Math.round(point[2] * 100000) / 100000}`
}

async function waterline(surface, cutter, z, sampling) {
  const wl = new Waterline()
  wl.setSTL(surface)
  wl.setCutter(cutter)
  wl.setZ(z)
  wl.setSampling(sampling)
  wl.run()
  let gcode = ''
  const loops = wl.getLoops()
  for (var i = 0; i < loops.length; i++) {
    const loop = loops[i];
    for (var j = 0; j < loop.length; j++) {
      const point = loop[j]
      gcode += 'G01 ' + pointToXYZ(point) + '\n'
    }
  }
  console.log(gcode)
}

async function adaptiveWaterline(surface, cutter, z, sampling, minSampling) {
  console.log('AdaptiveWaterline')
  const awl = new AdaptiveWaterline()
  awl.setSTL(surface)
  awl.setCutter(cutter)
  awl.setZ(z)
  awl.setSampling(sampling)
  awl.setMinSampling(minSampling)
  awl.run()
  let gcode = ''
  const loops = awl.getLoops()
  for (var i = 0; i < loops.length; i++) {
    const loop = loops[i]
    for (var j = 0; j < loop.length; j++) {
      const point = loop[j]
      gcode += 'G01 ' + pointToXYZ(point) + '\n'
    }
  }
  console.log(gcode)
}

async function pathDropCutter(surface, cutter, sampling, path) {
  console.log('PathDropCutter')
  const pdc = new PathDropCutter()
  pdc.setSTL(surface)
  pdc.setCutter(cutter)
  pdc.setPath(path)
  pdc.setZ(0)
  pdc.setSampling(sampling)
  pdc.run()
  const points = pdc.getCLPoints()
  let gcode = ''
  for (var j = 0; j < points.length; j++) {
    const point = points[j]
    gcode += 'G01 ' + pointToXYZ(point) + '\n'
  }
  console.log(gcode)
}

async function adaptivePathDropCutter(surface, cutter, sampling, minSampling, path) {
  console.log('AdaptivePathDropCutter')
  const apdc = new AdaptivePathDropCutter()
  apdc.setSTL(surface)
  apdc.setCutter(cutter)
  apdc.setPath(path)
  apdc.setZ(0)
  apdc.setSampling(sampling)
  apdc.setMinSampling(minSampling)
  apdc.run()
  const points = apdc.getCLPoints()
  let gcode = ''
  for (var j = 0; j < points.length; j++) {
    const point = points[j]
    gcode += 'G01 ' + pointToXYZ(point) + '\n'
  }
  console.log(gcode)
}

async function main() {
    const surface = new STLSurf()
    const stlContents = fs.readFileSync(__dirname + '/../../stl/gnu_tux_mod.stl') 
    new STLReader(stlContents, surface)
    const cutter = new CylCutter(4, 20)
    await waterline(surface, cutter, 1, 0.1)
    await adaptiveWaterline(surface, cutter, 1, 0.1, 0.001)
    const path = new Path()
    const p1 = new Point(-2, 4, 0)
    const p2 = new Point(11, 4, 0)
    const l = new Line(p1, p2)
    path.append(l)
    await pathDropCutter(surface, cutter, 0.1, path)
    await adaptivePathDropCutter(surface, cutter, 0.04, 0.01, path)
}

main()