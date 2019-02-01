import ocl
import camvtk
import time
import vtk


def filter_path(path,tol):
    f = ocl.LineCLFilter()
    f.setTolerance(tol)
    for p in path:
        p2 = ocl.CLPoint(p.x,p.y,p.z)
        f.addCLPoint(p2)
    f.run()
    return  f.getCLPoints()
    
    
def adaptive_path_drop_cutter(s, cutter, path):
    apdc = ocl.AdaptivePathDropCutter()
    apdc.setSTL(s)
    apdc.setCutter(cutter)
    # set the minimum Z-coordinate, or "floor" for drop-cutter
    #apdc.minimumZ = -1 
    apdc.setSampling(0.04)
    apdc.setMinSampling(0.0008)
    apdc.setPath( path )
    apdc.run()
    return apdc.getCLPoints()
    
def drawPaths(paths):
    ngc_writer.preamble()
    
    for path in cl_filtered_paths:
        ngc_writer.pen_up()
        first_pt = path[0]
        ngc_writer.xy_rapid_to( first_pt.x, first_pt.y )
        ngc_writer.pen_down( first_pt.z )
        for p in path[1:]:
            ngc_writer.line_to(p.x,p.y,p.z)

    ngc_writer.postamble()
    
if __name__ == "__main__":  
    print(ocl.version())
    
    myscreen = camvtk.VTKScreen()    
    #stl = camvtk.STLSurf("../stl/demo.stl")
    stl = camvtk.STLSurf("../stl/pycam-textbox.stl") 
    print("STL surface read")
    myscreen.addActor(stl)
    stl.SetWireframe()    
    polydata = stl.src.GetOutput()
    s= ocl.STLSurf()
    camvtk.vtkPolyData2OCLSTL(polydata, s)
    print("STLSurf with ", s.size(), " triangles")
    print(s.getBounds())
    # define a cutter
    cutter = ocl.CylCutter(10, 50) # diameter, length
    #cutter = ocl.BullCutter(0.6, 0.01, 5)
    print(cutter)
    #pdc = ocl.PathDropCutter()   # create a pdc
    apdc = ocl.AdaptivePathDropCutter()
    #pdc.setSTL(s)
    apdc.setSTL(s)
    #pdc.setCutter(cutter)               # set the cutter
    apdc.setCutter(cutter)
    #print "set minimumZ"
    #pdc.minimumZ = -1                   # set the minimum Z-coordinate, or "floor" for drop-cutter
    #apdc.minimumZ = -1 
    #print "set the sampling interval"
    #pdc.setSampling(0.4)
    apdc.setSampling(0.4)
    apdc.setMinSampling(0.0008)
    print(" apdc sampling = ", apdc.getSampling())
    
    
     
    ymin=0
    ymax=50
    Ny=40  # number of lines in the y-direction
    dy = float(ymax-ymin)/(Ny-1)  # the y step-over
    
    # create a simple "Zig" pattern where we cut only in one direction.
    paths = []
    # create a list of paths
    for n in range(0,Ny):
        path = ocl.Path() 
        y = ymin+n*dy           # current y-coordinate
        p1 = ocl.Point(0,y,0)   # start-point of line
        p2 = ocl.Point(130,y,0)   # end-point of line
        l = ocl.Line(p1,p2)     # line-object
        path.append( l )        # add the line to the path
        paths.append(path)
    
    cl_paths=[]
    
    # we now have a list of paths to run through apdc
    t_before = time.time()
    n_aclp=0
    for p in paths:
        aclp = adaptive_path_drop_cutter(s,cutter,p) # the output is a list of Cutter-Locations
        n_aclp = n_aclp + len(aclp)
        cl_paths.append(aclp)
    
    t_after = time.time()
    print("( OpenCamLib::AdaptivePathDropCutter run took %.2f s )" % ( t_after-t_before ))
    print("( got %d raw CL-points )" % ( n_aclp ))
    # to reduce the G-code size we filter here. (this is not strictly required and could be omitted)
    # we could potentially detect G2/G3 arcs here, if there was a filter for that.
    
    tol = 0.001    
    print("( filtering to tolerance %.4f )" % ( tol ) )
    cl_filtered_paths = []
    t_before = time.time()
    n_filtered=0
    for cl_path in cl_paths:
        cl_filtered = filter_path(cl_path,tol)
        n_filtered = n_filtered + len(cl_filtered)
        cl_filtered_paths.append(cl_filtered)
    t_after = time.time()
    calctime = t_after-t_before
    print("( got %d filtered CL-points. Filter done in %.3f s )" % ( n_filtered , calctime ))
    
    drawPaths(cl_filtered_paths)
    """
    # some parameters for this "zigzig" pattern    
    ymin=0
    ymax=50
    Ny=10  # number of lines in the y-direction
    dy = float(ymax-ymin)/Ny  # the y step-over
    print("step-over ",dy)
    #path = ocl.Path()                   # create an empty path object 
    path2 = ocl.Path() 
    # add Line objects to the path in this loop
    for n in range(0,Ny):
        y = ymin+n*dy
        p1 = ocl.Point(0,y,-100)   # start-point of line
        p2 = ocl.Point(130,y,-100)   # end-point of line
        #sl = ocl.Line(p1,p2)     # line-object
        l2 = ocl.Line(p1,p2)
        #path.append( l )        # add the line to the path
        path2.append( l2 )

    print(" set the path for pdf ")
    #pdc.setPath( path )
    apdc.setPath( path2 )
    
    #print " run the calculation "
    #t_before = time.time()
    #pdc.run()                   # run drop-cutter on the path
    #t_after = time.time()
    #print " pdc run took ", t_after-t_before," s"
    
    print(" run the calculation ")
    t_before = time.time()
    apdc.run()                   # run drop-cutter on the path
    t_after = time.time()
    print(" apdc run took ", t_after-t_before," s")
    
    
    print("get the results ")
    #clp = pdc.getCLPoints()     # get the cl-points from pdf
    aclp = apdc.getCLPoints()
    print("got ", len(aclp) ," adaptive points")
    
    #aclp_lifted=[]
    #for p in aclp:
    #    p2 = ocl.Point(p.x,p.y,p.z) + ocl.Point(0,0,1)
    #    aclp_lifted.append(p2)
    
    # filter the adaptively sampled toolpaths
    
    print("filtering. before filter we have", len(aclp),"cl-points")
    t_before = time.time()
    f = ocl.LineCLFilter()
    f.setTolerance(0.001)
    for p in aclp:
        p2 = ocl.CLPoint(p.x,p.y,p.z)
        f.addCLPoint(p2)
        
    f.run()
    t_after = time.time()
    calctime = t_after-t_before
    print(" done in ", calctime," s")
    
    cl_filtered = f.getCLPoints()
    #aclp_lifted2=[]
    #for p in cl_filtered:
    #    p2 = ocl.Point(p.x,p.y,p.z) + ocl.Point(0,0,1)
    #    aclp_lifted2.append(p2)
    
    
    print(" render the CL-points")
    #camvtk.drawCLPointCloud(myscreen, clp)
    
    camvtk.drawCLPointCloud(myscreen, cl_filtered)
    
    for p in cl_filtered:
        myscreen.
    """
    
    #camvtk.drawCLPointCloud(myscreen, aclp_lifted2)
    #myscreen.addActor( camvtk.PointCloud(pointlist=clp, collist=ccp)  )
    myscreen.camera.SetPosition(3, 23, 15)
    myscreen.camera.SetFocalPoint(5, 5, 0)
    myscreen.render()
    print(" All done.")
    myscreen.iren.Start()
