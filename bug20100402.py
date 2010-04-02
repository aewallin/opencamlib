import ocl as cam

# this illustrates issue 8

if __name__ == "__main__":
   cutter = cam.BallCutter(3.0)
   path = cam.Path()
   path.append(cam.Line(cam.Point(-5.0, 1.2, 0), cam.Point(5.0, 1.2, 0)))
   s=cam.STLSurf("stl/sphere2.stl")
   dcf = cam.PathDropCutterFinish(s)
   dcf.setCutter(cutter)
   dcf.setPath(path)
   dcf.run()
