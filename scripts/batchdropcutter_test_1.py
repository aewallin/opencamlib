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

if __name__ == "__main__":  
    myscreen = camvtk.VTKScreen()
    
    stl = camvtk.STLSurf("../stl/gnu_tux_mod.stl")
    
    myscreen.addActor(stl)
    stl.SetWireframe()
    stl.SetColor((0.5,0.5,0.5))
	
    polydata = stl.src.GetOutput()
    s = cam.STLSurf()
    camvtk.vtkPolyData2OCLSTL(polydata, s)
    print "STL surface read ", s.size(), " triangles"
    #cutter = cam.BallCutter(1)
    cutter = cam.CylCutter(1.234)
    #print cc.type
    minx=0
    dx=0.1/5
    maxx=9
    miny=-0.2
    dy=1
    maxy=12.2
    z=-0.2
    clpoints = CLPointGrid(minx,dx,maxx,miny,dy,maxy,z)
    print "generated grid with", len(clpoints)," CL-points"
    """
    print "for-loop...",
    t_before = time.time()
    for cl in clpoints:
        cc = cam.CCPoint()
        cutter.dropCutterSTL(cl,cc,s)
    t_after = time.time()
    print " done in ", t_after-t_before," s"
    """
    
    # batchdropcutter
    
    bdc = cam.BatchDropCutter()
    bdc.setSTL(s,1)
    bdc.setCutter(cutter)
    for p in clpoints:
        bdc.appendPoint(p)
    
    t_before = time.time()    
    bdc.nthreads=3
    bdc.dropCutter4()
    t_after = time.time()
    print " done in ", t_after-t_before," s"
    
    #print "none=",nn," vertex=",nv, " edge=",ne, " facet=",nf, " sum=", nn+nv+ne+nf
    print len(clpoints), " cl points evaluated"
    
    clpoints = bdc.getCLPoints()
    
    print "rendering...",
    drawCLpoints(myscreen, clpoints)
    print "done"
    
    myscreen.camera.SetPosition(3, 23, 15)
    myscreen.camera.SetFocalPoint(4, 5, 0)
    myscreen.render()
    
    w2if = vtk.vtkWindowToImageFilter()
    w2if.SetInput(myscreen.renWin)
    lwr = vtk.vtkPNGWriter()
    lwr.SetInput( w2if.GetOutput() )
    w2if.Modified()
    lwr.SetFileName("tux1.png")
    #lwr.Write()
    
    t = camvtk.Text()
    t.SetPos( (myscreen.width-200, myscreen.height-30) )
    myscreen.addActor( t)
    
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
        


    myscreen.iren.Start()
    raw_input("Press Enter to terminate") 
    
