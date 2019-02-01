import ocl
import camvtk
import time
import vtk


if __name__ == "__main__":  
    print(ocl.version())
    
    myscreen = camvtk.VTKScreen()    
    stl = camvtk.STLSurf("../stl/gnu_tux_mod.stl")
    print("STL surface read")
    myscreen.addActor(stl)
    stl.SetWireframe()    
    polydata = stl.src.GetOutput()
    s= ocl.STLSurf()
    camvtk.vtkPolyData2OCLSTL(polydata, s)
    print("STLSurf with ", s.size(), " triangles")
    
    # define a cutter
    cutter = ocl.CylCutter(0.6, 5)
    print(cutter)
    
    print("creating PathDropCutter()")
    pdc = ocl.PathDropCutter()   # create a pdc
    print("set STL surface")
    pdc.setSTL(s)
    print("set cutter")
    pdc.setCutter(cutter)               # set the cutter
    print("set minimumZ")
    pdc.minimumZ = -1                   # set the minimum Z-coordinate, or "floor" for drop-cutter
    print("set the sampling interval")
    pdc.setSampling(0.0123)
    
    # some parameters for this "zigzig" pattern    
    ymin=0
    ymax=12
    Ny=40  # number of lines in the y-direction
    dy = float(ymax-ymin)/Ny  # the y step-over
    
    path = ocl.Path()                   # create an empty path object 
    # add Line objects to the path in this loop
    for n in range(0,Ny):
        y = ymin+n*dy
        p1 = ocl.Point(0,y,0)   # start-point of line
        p2 = ocl.Point(9,y,0)   # end-point of line
        l = ocl.Line(p1,p2)     # line-object
        path.append( l )        # add the line to the path

    print(" set the path for pdf ")
    pdc.setPath( path )
    
    print(" run the calculation ")
    t_before = time.time()
    pdc.run()                   # run drop-cutter on the path
    t_after = time.time()
    print("run took ", t_after-t_before," s")
    
    print("get the results ")
    clp = pdc.getCLPoints()     # get the cl-points from pdf
    
    print(" render the CL-points")
    camvtk.drawCLPointCloud(myscreen, clp)
    #myscreen.addActor( camvtk.PointCloud(pointlist=clp, collist=ccp)  )
    myscreen.camera.SetPosition(3, 23, 15)
    myscreen.camera.SetFocalPoint(5, 5, 0)
    myscreen.render()
    print(" All done.")
    myscreen.iren.Start()
