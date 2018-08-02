import ocl
import pyocl
import camvtk
import time
import vtk
import datetime       
import math

if __name__ == "__main__": 
    print ocl.version()    
    myscreen = camvtk.VTKScreen()
    #stl = camvtk.STLSurf("../stl/Cylinder_1.stl")
    stl = camvtk.STLSurf("../../stl/gnu_tux_mod.stl")
    #stl = camvtk.STLSurf("../stl/demo.stl")
    myscreen.addActor(stl)
    stl.SetWireframe()
    stl.SetColor((0.5,0.5,0.5))
    
    polydata = stl.src.GetOutput()
    s = ocl.STLSurf()
    camvtk.vtkPolyData2OCLSTL(polydata, s)
    print "STL surface read,", s.size(), "triangles"
    
    angle = math.pi/4
    diameter=1.77321
    length=5
    #cutter = ocl.BallCutter(diameter, length)
    cutter = ocl.CylCutter(diameter, length)
    #cutter = ocl.BullCutter(diameter, 0.2, length)
    #cutter = ocl.ConeCutter(diameter, angle, length)
    #cutter = cutter.offsetCutter( 0.4 )
    
    print cutter
    
    minx=-1
    dx=0.1/5
    
    maxx=10
    miny=-1
    dy=1/float(2)
    maxy=13
    z=-1
    # this generates a list of CL-points in a grid
    clpoints = pyocl.CLPointGrid(minx,dx,maxx,miny,dy,maxy,z)
    print "generated grid with", len(clpoints)," CL-points"
    
    # batchdropcutter    
    bdc = ocl.BatchDropCutter()
    #bdc.setThreads(1)
    print "bdc()"
    bdc.setBucketSize(2)
    print "bucketSize"
    bdc.setSTL(s)
    print "setSTL()"
    bdc.setCutter(cutter)
    print "setCutter()"
    #bdc.setThreads(1)  # explicitly setting one thread is better for debugging
    for p in clpoints:
        bdc.appendPoint(p)
    print "bdc has ",len(clpoints)," cl-points"
    
    t_before = time.time()    
    bdc.run() # run the actual drop-cutter
    dc_calls = bdc.getCalls()
    t_after = time.time()
    calctime = t_after-t_before
    print " BDC done in ", calctime," s", dc_calls," dc-calls" 
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
    print " done in ", calctime," s"
    
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
    
    #w2if = vtk.vtkWindowToImageFilter()
    #w2if.SetInput(myscreen.renWin)
    #lwr = vtk.vtkPNGWriter()
    #lwr.SetInput( w2if.GetOutput() )
    #w2if.Modified()
    #lwr.SetFileName("tux1.png")
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
    
