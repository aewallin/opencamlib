import ocl
import camvtk
import time
import vtk
import datetime


def CLPointGrid(minx,dx,maxx,miny,dy,maxy,z):
    plist = []
    xvalues = [round(minx+n*dx,2) for n in xrange(int(round((maxx-minx)/dx))+1) ]
    yvalues = [round(miny+n*dy,2) for n in xrange(int(round((maxy-miny)/dy))+1) ]
    for y in yvalues:
        for x in xvalues:
            plist.append( ocl.Point(x,y,z) )
    return plist
        
        
if __name__ == "__main__":  
    myscreen = camvtk.VTKScreen()
    
    #stl = camvtk.STLSurf("../stl/gnu_tux_mod.stl")
    stl = camvtk.STLSurf("../stl/demo.stl")
    myscreen.addActor(stl)
    stl.SetWireframe()
    stl.SetColor((0.5,0.5,0.5))
    
    polydata = stl.src.GetOutput()
    s = ocl.STLSurf()
    camvtk.vtkPolyData2OCLSTL(polydata, s)
    print "STL surface read ", s.size(), " triangles"
    
    cutter = ocl.BallCutter(1.4321)
    #cutter = ocl.CylCutter(1.123)
    #cutter = ocl.BullCutter(1.123, 0.2)
    
    print ocl.revision()    
    print cutter
    
    minx=0
    dx=0.1/6
    maxx=10
    miny=0
    dy=1
    maxy=10
    z=-17
    clpoints = CLPointGrid(minx,dx,maxx,miny,dy,maxy,z)
    print "generated grid with", len(clpoints)," CL-points"
    
    # batchdropcutter    
    bdc1 = ocl.BatchDropCutter()
    bdc1.setSTL(s,1)
    bdc1.setCutter(cutter)
    for p in clpoints:
        bdc1.appendPoint(p)
    
    t_before = time.time()    
    print "threads=",bdc1.nthreads
    bdc1.dropCutter4()
    t_after = time.time()
    calctime = t_after-t_before
    print " done in ", calctime," s"
    
    clpoints = bdc1.getCLPoints()
    ccpoints = bdc1.getCCPoints()
    print len(clpoints), " cl points evaluated"
    print len(ccpoints), " cc-points"
    #exit()
    
    print "rendering...",
    point_actor=camvtk.PointCloud( pointlist=clpoints, collist=ccpoints) 
    point_actor.SetPoints()
    myscreen.addActor(point_actor )
    
    print "done"
        
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
    stltext = "%i triangles\n%i CL-points\n%0.1f seconds" % (s.size(), len(clpoints), calctime)
    t2.SetText(stltext)
    t2.SetPos( (50, myscreen.height-200) )
    myscreen.addActor( t2)
    
    t3 = camvtk.Text()
    ctext = "Cutter: %s" % ( str(cutter) )
    
    t3.SetText(ctext)
    t3.SetPos( (50, myscreen.height-250) )
    myscreen.addActor( t3)

    myscreen.render()
    myscreen.iren.Start()
    raw_input("Press Enter to terminate") 
    
