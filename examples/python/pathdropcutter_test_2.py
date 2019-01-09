import ocl
import camvtk
import time
import vtk


if __name__ == "__main__":  
    print ocl.version()
    
    myscreen = camvtk.VTKScreen()    
    stl = camvtk.STLSurf("../stl/demo.stl")
    #stl = camvtk.STLSurf("../stl/pycam-textbox.stl") 
    print "STL surface read"
    myscreen.addActor(stl)
    stl.SetWireframe()    
    polydata = stl.src.GetOutput()
    s= ocl.STLSurf()
    camvtk.vtkPolyData2OCLSTL(polydata, s)
    print "STLSurf with ", s.size(), " triangles"
    
    # define a cutter
    #cutter = ocl.CylCutter(0.6, 5)
    cutter = ocl.BullCutter(0.6, 0.01, 5)
    print cutter
    pdc = ocl.PathDropCutter()   # create a pdc
    apdc = ocl.AdaptivePathDropCutter()
    pdc.setSTL(s)
    apdc.setSTL(s)
    pdc.setCutter(cutter)               # set the cutter
    apdc.setCutter(cutter)
    #print "set minimumZ"
    #pdc.minimumZ = -1                   # set the minimum Z-coordinate, or "floor" for drop-cutter
    #apdc.minimumZ = -1 
    #print "set the sampling interval"
    pdc.setSampling(0.4)
    apdc.setSampling(0.4)
    apdc.setMinSampling(0.0008)
    print " apdc sampling = ", apdc.getSampling()
    # some parameters for this "zigzig" pattern    
    ymin=0
    ymax=12
    Ny=10  # number of lines in the y-direction
    dy = float(ymax-ymin)/Ny  # the y step-over
    
    path = ocl.Path()                   # create an empty path object 
    path2 = ocl.Path() 
    # add Line objects to the path in this loop
    for n in xrange(0,Ny):
        y = ymin+n*dy
        p1 = ocl.Point(0,y,0)   # start-point of line
        p2 = ocl.Point(10,y,0)   # end-point of line
        l = ocl.Line(p1,p2)     # line-object
        l2 = ocl.Line(p1,p2)
        path.append( l )        # add the line to the path
        path2.append( l2 )

    print " set the path for pdf "
    pdc.setPath( path )
    apdc.setPath( path2 )
    
    print " run the calculation "
    t_before = time.time()
    pdc.run()                   # run drop-cutter on the path
    t_after = time.time()
    print " pdc run took ", t_after-t_before," s"
    
    print " run the calculation "
    t_before = time.time()
    apdc.run()                   # run drop-cutter on the path
    t_after = time.time()
    print " apdc run took ", t_after-t_before," s"
    
    
    print "get the results "
    clp = pdc.getCLPoints()     # get the cl-points from pdf
    aclp = apdc.getCLPoints()
    print "got ", len(aclp) ," adaptive points"
    
    aclp_lifted=[]
    for p in aclp:
        p2 = ocl.Point(p.x,p.y,p.z) + ocl.Point(0,0,1)
        aclp_lifted.append(p2)
    
    # filter the adaptively sampled toolpaths
    
    print "filtering. before filter we have", len(aclp_lifted),"cl-points"
    t_before = time.time()
    f = ocl.LineCLFilter()
    f.setTolerance(0.001)
    for p in aclp_lifted:
        p2 = ocl.CLPoint(p.x,p.y,p.z)
        f.addCLPoint(p2)
        
    f.run()
    t_after = time.time()
    calctime = t_after-t_before
    print " done in ", calctime," s"
    cl_filtered = f.getCLPoints()
    print "       after filter we have", len(cl_filtered),"cl-points"
    
    aclp_lifted2=[]
    for p in cl_filtered:
        p2 = ocl.Point(p.x,p.y,p.z) + ocl.Point(0,0,1)
        aclp_lifted2.append(p2)
    
    
    print " render the CL-points"
    camvtk.drawCLPointCloud(myscreen, clp)
    camvtk.drawCLPointCloud(myscreen, aclp_lifted)
    camvtk.drawCLPointCloud(myscreen, aclp_lifted2)
    #myscreen.addActor( camvtk.PointCloud(pointlist=clp, collist=ccp)  )
    myscreen.camera.SetPosition(3, 23, 15)
    myscreen.camera.SetFocalPoint(5, 5, 0)
    myscreen.render()
    print " All done."
    myscreen.iren.Start()
