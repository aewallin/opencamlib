import {
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
} from '@opencamlib/opencamlib'

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
  const loops = await wl.getLoops()
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
  const loops = await awl.getLoops()
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
  pdc.setSampling(sampling)
  pdc.run()
  const points = await pdc.getPoints()
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
  apdc.setSampling(sampling)
  apdc.setMinSampling(minSampling)
  apdc.run()
  const points = await apdc.getPoints()
  let gcode = ''
  for (var j = 0; j < points.length; j++) {
    const point = points[j]
    gcode += 'G01 ' + pointToXYZ(point) + '\n'
  }
  console.log(gcode)
}


fetch('gnu_tux_mod.stl')
  .then(res => res.text())
  .then(async (stlContents) => {
    const surface = new STLSurf()
    new STLReader(stlContents, surface)
    const cutter = new CylCutter(4, 20)
    await waterline(surface, cutter, 1, 0.1)
    await adaptiveWaterline(surface, cutter, 1, 0.1, 0.001)
    const path = new Path()
    const p1 = new Point(0, 1, 0)
    const p2 = new Point(10, 1, 0)
    const l = new Line(p1, p2)
    path.append(l)
    await pathDropCutter(surface, cutter, 0.1, path)
    await adaptivePathDropCutter(surface, cutter, 0.04, 0.01, path)
})

// import '@kitware/vtk.js/Rendering/Profiles/Geometry';
// import vtkActor from '@kitware/vtk.js/Rendering/Core/Actor';
// import vtkConeSource from '@kitware/vtk.js/Filters/Sources/ConeSource';
// import vtkMapper from '@kitware/vtk.js/Rendering/Core/Mapper';
// import vtkOpenGLRenderWindow from '@kitware/vtk.js/Rendering/OpenGL/RenderWindow';
// import vtkRenderWindow from '@kitware/vtk.js/Rendering/Core/RenderWindow';
// import vtkRenderWindowInteractor from '@kitware/vtk.js/Rendering/Core/RenderWindowInteractor';
// import vtkRenderer from '@kitware/vtk.js/Rendering/Core/Renderer';
// import vtkInteractorStyleTrackballCamera from '@kitware/vtk.js/Interaction/Style/InteractorStyleTrackballCamera';

// const renderWindow = vtkRenderWindow.newInstance();
// const renderer = vtkRenderer.newInstance({ background: [0, 0, 0] });
// renderWindow.addRenderer(renderer);

// const coneSource = vtkConeSource.newInstance({ height: 1.0 });
// const mapper = vtkMapper.newInstance();
// mapper.setInputConnection(coneSource.getOutputPort());

// const actor = vtkActor.newInstance();
// actor.setMapper(mapper);
// renderer.addActor(actor);
// renderer.resetCamera();
// const openglRenderWindow = vtkOpenGLRenderWindow.newInstance();
// renderWindow.addView(openglRenderWindow);
// const container = document.createElement('div');
// document.querySelector('body')!.appendChild(container);
// openglRenderWindow.setContainer(container);
// const { width, height } = container.getBoundingClientRect();
// openglRenderWindow.setSize(width, height);
// const interactor = vtkRenderWindowInteractor.newInstance();
// interactor.setView(openglRenderWindow);
// interactor.initialize();
// interactor.bindEvents(container);
// interactor.setInteractorStyle(vtkInteractorStyleTrackballCamera.newInstance());

