import ocl as cam
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
            plist.append( cam.Point(x,y,z) )
    return plist
    
def drawCLpoints(myscreen, clpoints):
    for p in clpoints:
        myscreen.addActor( camvtk.Point(center=(p.x,p.y,p.z)) )


def drawCCpoints(myscreen, ccpoints):
    for p in ccpoints:
        myscreen.addActor( camvtk.Point(center=(p.x,p.y,p.z), color=camvtk.ccColor(p) ) )
        
        
if __name__ == "__main__":  
    myscreen = camvtk.VTKScreen()
    
    #stl = camvtk.STLSurf("../stl/gnu_tux_mod.stl")
    stl = camvtk.STLSurf("../stl/demo.stl")
    myscreen.addActor(stl)
    stl.SetWireframe()
    stl.SetColor((0.5,0.5,0.5))
    
    polydata = stl.src.GetOutput()
    s = cam.STLSurf()
    camvtk.vtkPolyData2OCLSTL(polydata, s)
    print "STL surface read ", s.size(), " triangles"
    
    #cutter = cam.BallCutter(1)
    
    cutter = cam.CylCutter(1.123)
    
    #cutter = cam.BullCutter(1.123, 0.2)
    
    print cutter.str()
    #print cc.type
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
    bdc = cam.BatchDropCutter()
    bdc.setSTL(s,1)
    bdc.setCutter(cutter)
    for p in clpoints:
        bdc.appendPoint(p)
    
    t_before = time.time()    
    print "threads=",bdc.nthreads
    bdc.dropCutter3()
    t_after = time.time()
    calctime = t_after-t_before
    print " done in ", calctime," s"
    
    #print "none=",nn," vertex=",nv, " edge=",ne, " facet=",nf, " sum=", nn+nv+ne+nf
    
    
    clpoints = bdc.getCLPoints()
    ccpoints = bdc.getCCPoints()
    print len(clpoints), " cl points evaluated"
    print len(ccpoints), " cc-points"
    print "rendering...",
    drawCLpoints(myscreen, clpoints)
    drawCCpoints(myscreen, ccpoints)
    print "done"
    
    """
    maxz=0
    maxp=0
    for p in ccpoints:
        if maxz < p.z:
            maxz = p.z
            maxp = p
    print "max z was:", maxz, " at ", maxp.str()
    """
    
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
    t.SetText("OpenCAMLib 10.04")
    t.SetPos( (myscreen.width-200, myscreen.height-30) )
    myscreen.addActor( t)
    
    t2 = camvtk.Text()
    stltext = "%i triangles\n%i CL-points\n%0.1f seconds" % (s.size(), len(clpoints), calctime)
    t2.SetText(stltext)
    t2.SetPos( (50, myscreen.height-200) )
    myscreen.addActor( t2)
    
    t3 = camvtk.Text()
    ctext = "Cutter: %s" % ( cutter.str() )
    
    t3.SetText(ctext)
    t3.SetPos( (50, myscreen.height-250) )
    myscreen.addActor( t3)
    
    """ 
    for n in range(1,360):
        t.SetText(datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S"))
        myscreen.camera.Azimuth( 1 )
        time.sleep(0.01)
        myscreen.render()
        lwr.SetFileName("frame"+ ('%03d' % n)+".png")
        w2if.Modified() 
        #lwr.Write()
    """
        

    myscreen.render()
    myscreen.iren.Start()
    raw_input("Press Enter to terminate") 
    
