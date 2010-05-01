import ocl as cam
import camvtk
import time
import vtk
import datetime
import math


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

def drawPoints(myscreen, clpoints, ccpoints):
    c=camvtk.PointCloud( pointlist=clpoints, collist=ccpoints) 
    c.SetPoints()
    myscreen.addActor(c )
            
        
if __name__ == "__main__":  
    myscreen = camvtk.VTKScreen()
    
    #stl = camvtk.STLSurf("../stl/gnu_tux_mod.stl")
    #stl = camvtk.STLSurf("../stl/beet_mm.stl")
    stl = camvtk.STLSurf("../stl/demo.stl")
    myscreen.addActor(stl)
    stl.SetWireframe()
    stl.SetColor((0.5,0.5,0.5))
    
    polydata = stl.src.GetOutput()
    s = cam.STLSurf()
    camvtk.vtkPolyData2OCLSTL(polydata, s)
    print "STL surface read ", s.size(), " triangles"
    
    #cutter = cam.BallCutter(1.4321)
    
    #cutter = cam.CylCutter(1.123)
    
    cutter = cam.BullCutter(1.123, 0.2)
    
    #cutter = cam.ConeCutter(0.43, math.pi/7)
    
    print cutter.str()
    #print cc.type
    minx=0
    dx=0.01
    maxx=10
    miny=0
    dy=1
    maxy=10
    z=-5
    clpoints = CLPointGrid(minx,dx,maxx,miny,dy,maxy,z)
    clpoints2 = CLPointGrid(minx,dx,maxx,miny,dy,maxy,z)
    print "generated grid with", len(clpoints)," CL-points"
    
    # batchdropcutter    
    bdc1 = cam.BatchDropCutter()
    bdc2 = cam.BatchDropCutter()
    bdc1.setSTL(s,1)
    bdc2.setSTL(s,1)
    bdc1.setCutter(cutter)
    bdc2.setCutter(cutter)
    
    for p in clpoints:
        bdc1.appendPoint(p)
    for p in clpoints2:
        bdc2.appendPoint(p)
    

    
    t_before = time.time()    
    bdc2.dropCutter4()
    t_after = time.time()
    calctime = t_after-t_before
    print " done in ", calctime," s"    
    
    #print "none=",nn," vertex=",nv, " edge=",ne, " facet=",nf, " sum=", nn+nv+ne+nf
    
    
    cl1 = bdc1.getCLPoints()
    cc1 = bdc1.getCCPoints()
    cl2 = bdc2.getCLPoints()
    cc2 = bdc2.getCCPoints()
    
   
    #exit()
    print "rendering...",
    #drawPoints(myscreen, cc2, cc2)
    drawPoints(myscreen, cc2, cc2)

    print "done"
       
    myscreen.camera.SetPosition(3, 23, 15)
    myscreen.camera.SetFocalPoint(4, 5, 0)
    
    t = camvtk.Text()
    t.SetText("OpenCAMLib 10.04")
    t.SetPos( (myscreen.width-200, myscreen.height-30) )
    myscreen.addActor( t)
    
    t2 = camvtk.Text()
    stltext = "%i triangles\n%i CL-points\n%0.1f seconds" % (s.size(), len(cl2), calctime)
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
    
