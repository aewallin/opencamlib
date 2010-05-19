import ocl
import camvtk
import time
import vtk


if __name__ == "__main__":  
    print ocl.revision()
    myscreen = camvtk.VTKScreen()    
    stl = camvtk.STLSurf("../stl/gnu_tux_mod.stl")
    print "STL surface read"
    myscreen.addActor(stl)
    stl.SetWireframe()    
    polydata = stl.src.GetOutput()
    s= ocl.STLSurf()
    camvtk.vtkPolyData2OCLSTL(polydata, s)
    print "STLSurf with ", s.size(), " triangles"
    
    # define a cutter
    cutter = ocl.CylCutter(0.6)
    print cutter
    
    pdf = ocl.PathDropCutterFinish(s)   # create a pdf-object for the surface s
    pdf.setCutter(cutter)               # set the cutter
    pdf.minimumZ = -1                   # set the minimum Z-coordinate, or "floor" for drop-cutter

    # some parameters for this "zigzig" pattern    
    ymin=0
    ymax=12
    Ny=40  # number of lines in the y-direction
    dy = float(ymax-ymin)/Ny  # the y step-over
    
    path = ocl.Path()                   # create an empty path object 
    # add Line objects to the path in this loop
    for n in xrange(0,Ny):
        y = ymin+n*dy
        p1 = ocl.Point(0,y,0)   # start-point of line
        p2 = ocl.Point(9,y,0)   # end-point of line
        l = ocl.Line(p1,p2)     # line-object
        path.append( l )        # add the line to the path

    # set the path for pdf
    pdf.setPath( path )
    
    t_before = time.time()
    pdf.run()                   # run drop-cutter on the path
    t_after = time.time()
    print "run took ", t_after-t_before," s"
    
    clp = pdf.getCLPoints()     # get the cl-points from pdf
    ccp = pdf.getCCPoints()     # get the cc-points from pdf
    
    # render the CL-points
    myscreen.addActor( camvtk.PointCloud(pointlist=clp, collist=ccp)  )
    myscreen.camera.SetPosition(3, 23, 15)
    myscreen.camera.SetFocalPoint(5, 5, 0)
    myscreen.render()
    myscreen.iren.Start()
