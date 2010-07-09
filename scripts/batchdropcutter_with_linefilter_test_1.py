import ocl
import pyocl
import camvtk
import time
import vtk
import datetime       

if __name__ == "__main__": 
    print ocl.revision()    
    myscreen = camvtk.VTKScreen()
    
    stl = camvtk.STLSurf("../stl/gnu_tux_mod.stl")
    #stl = camvtk.STLSurf("../stl/demo.stl")
    myscreen.addActor(stl)
    stl.SetWireframe()
    stl.SetColor((0.5,0.5,0.5))
    
    polydata = stl.src.GetOutput()
    s = ocl.STLSurf()
    camvtk.vtkPolyData2OCLSTL(polydata, s)
    print "STL surface read,", s.size(), "triangles"
    
    #cutter = ocl.BallCutter(1.4321)
    #cutter = ocl.CylCutter(1.123)
    cutter = ocl.BullCutter(1.123, 0.2)
    print cutter
    print "radius=",cutter.radius
    
    minx=0
    dx=0.1/10
    maxx=10
    miny=0
    dy=1
    maxy=10
    z=-1
    # this generates a list of CL-points in a grid
    clpoints = pyocl.CLPointGrid(minx,dx,maxx,miny,dy,maxy,z)
    print "generated grid with", len(clpoints)," CL-points"
    
    # batchdropcutter    
    bdc = ocl.BatchDropCutter()
    bdc.setSTL(s,1)
    bdc.setCutter(cutter)
    #bdc.setThreads(1)  # explicitly setting one thread is better for debugging
    for p in clpoints:
        bdc.appendPoint(p)
    
    t_before = time.time()    
    bdc.dropCutter4()
    dc_calls = bdc.dcCalls
    t_after = time.time()
    calctime = t_after-t_before
    print " done in ", calctime," s"
    dropcutter_time = calctime
    clpoints = bdc.getCLPoints()

    #print len(clpoints), " cl points evaluated"
    
    print "rendering raw CL-points."
    
    # draw the CL-points
    camvtk.drawCLPointCloud(myscreen, clpoints)
    
    
    # filter
    print "filtering. before filter we have", len(clpoints),"cl-points"
    t_before = time.time()
    f = ocl.LineCLFilter()
    f.setTolerance(0.001)
    for p in clpoints:
        f.addCLPoint(p)
    f.run()
    t_after = time.time()
    calctime = t_after-t_before
    print " filter done in ", calctime," s"
    
    clp2 = f.getCLPoints()
    print "after filtering we have", len(clp2),"cl-points"
    
    # draw the filtered points
    # offset these points up for clarity
    for p in clp2:
        p.z=p.z+3
    
    print "rendering filtered CL-points."
    camvtk.drawCLPointCloud(myscreen, clp2)
    
    
    print "all done."
        
    myscreen.camera.SetPosition(3, 23, 15)
    myscreen.camera.SetFocalPoint(4, 5, 0)
    
    
    w2if = vtk.vtkWindowToImageFilter()
    w2if.SetInput(myscreen.renWin)
    lwr = vtk.vtkPNGWriter()
    lwr.SetInput( w2if.GetOutput() )
    w2if.Modified()
    lwr.SetFileName("tux1.png")
    #lwr.Write()
    
    t = camvtk.Text()
    t.SetText("OpenCAMLib")
    t.SetPos( (myscreen.width-200, myscreen.height-30) )
    myscreen.addActor( t)
    
    t2 = camvtk.Text()
    stltext = "%s\n%i triangles\n%i CL-points\n%i DropCutter() calls\n%0.1f seconds\n%0.3f us/call\n%i filtered CL-points"  \
               % ( str(cutter), s.size(), len(clpoints), dc_calls, dropcutter_time, 1e6* dropcutter_time/dc_calls, len(clp2) ) 
    t2.SetText(stltext)
    t2.SetPos( (50, myscreen.height-200) )
    myscreen.addActor( t2)
    


    myscreen.render()
    myscreen.iren.Start()
    raw_input("Press Enter to terminate") 
    
