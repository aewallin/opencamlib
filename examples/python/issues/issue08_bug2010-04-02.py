import ocl

# this illustrates issue 8

if __name__ == "__main__":
    print(ocl.version())
    cutter = ocl.CylCutter(3.0, 6)
    path = ocl.Path()
    path.append(ocl.Line(ocl.Point(-6.51, 0, 0), ocl.Point(6.51, 1.2, 0)))
    s=ocl.STLSurf()
    ocl.STLReader("../../stl/sphere2.stl",s)
    pdc = ocl.PathDropCutter()
    pdc.setSTL(s)
    pdc.setCutter(cutter)
    pdc.setPath(path)
    pdc.run()
    clpts = pdc.getCLPoints()
    for p in clpts:
        print(p)
