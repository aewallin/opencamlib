import ocl

# this illustrates issue 8

if __name__ == "__main__":
   cutter = ocl.CylCutter(3.0)
   path = ocl.Path()
   path.append(ocl.Line(ocl.Point(-6.51, 0, 0), ocl.Point(6.51, 1.2, 0)))
   s=ocl.STLSurf("../stl/sphere2.stl")
   dcf = ocl.PathDropCutterFinish(s)
   dcf.setCutter(cutter)
   dcf.setPath(path)
   dcf.run()
