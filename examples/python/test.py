from opencamlib import ocl

def pointToXYZ(point):
    return "X{:.5f}".format(point.x).rstrip('0').rstrip('.') + " Y{:.5f}".format(point.y).rstrip('0').rstrip('.') + " Z{:.5f}".format(point.z).rstrip('0').rstrip('.')

def waterline(surface, cutter, z, sampling):
    wl = ocl.Waterline()
    wl.setSTL(surface)
    wl.setCutter(cutter)
    wl.setZ(z)
    wl.setSampling(sampling)
    wl.run()
    gcode = ''
    loops = wl.getLoops()
    for loop in loops:
        for point in loop:
            gcode += "G01 " + pointToXYZ(point) + "\n"
    print(gcode)

def adaptiveWaterline(surface, cutter, z, sampling, minSampling):
    print('AdaptiveWaterline')
    awl = ocl.AdaptiveWaterline()
    awl.setSTL(surface)
    awl.setCutter(cutter)
    awl.setZ(z)
    awl.setSampling(sampling)
    awl.setMinSampling(minSampling)
    awl.run()
    gcode = ''
    loops = awl.getLoops()
    for loop in loops:
        for point in loop:
            gcode += "G01 " + pointToXYZ(point) + "\n"
    print(gcode)

def pathDropCutter(surface, cutter, sampling, path):
    print('PathDropCutter')
    pdc = ocl.PathDropCutter()
    pdc.setSTL(surface)
    pdc.setCutter(cutter)
    pdc.setPath(path)
    pdc.setZ(0)
    pdc.setSampling(sampling)
    pdc.run()
    points = pdc.getCLPoints()
    gcode = ''
    for point in points:
        gcode += "G01 " + pointToXYZ(point) + "\n"
    print(gcode)

def adaptivePathDropCutter(surface, cutter, sampling, minSampling, path):
    print('AdaptivePathDropCutter')
    apdc = ocl.AdaptivePathDropCutter()
    apdc.setSTL(surface)
    apdc.setCutter(cutter)
    apdc.setPath(path)
    apdc.setZ(0)
    apdc.setSampling(sampling)
    apdc.setMinSampling(minSampling)
    apdc.run()
    points = apdc.getCLPoints()
    gcode = ''
    for point in points:
        gcode += "G01 " + pointToXYZ(point) + "\n"
    print(gcode)

if __name__ == "__main__":  
    print(ocl.version())
    surface = ocl.STLSurf()
    ocl.STLReader("../../stl/gnu_tux_mod.stl", surface)
    cutter = ocl.CylCutter(4, 20)
    waterline(surface, cutter, 1, 0.1)
    adaptiveWaterline(surface, cutter, 1, 0.1, 0.001)
    path = ocl.Path()
    p1 = ocl.Point(-2, 4, 0)
    p2 = ocl.Point(11, 4, 0)
    l = ocl.Line(p1, p2)
    path.append(l)
    pathDropCutter(surface, cutter, 0.1, path)
    adaptivePathDropCutter(surface, cutter, 0.04, 0.01, path)
    print("done.")
