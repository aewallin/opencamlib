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
    
def drawPoints(myscreen, clpoints, ccpoints):
    c=camvtk.PointCloud( pointlist=clpoints, collist=ccpoints) 
    c.SetPoints()
    myscreen.addActor(c )
            
        
if __name__ == "__main__":  
    myscreen = camvtk.VTKScreen()
    
    #stl = camvtk.STLSurf("../stl/gnu_tux_mod.stl")
    stl = camvtk.STLSurf("../stl/mount_rush.stl")
    myscreen.addActor(stl)
    stl.SetWireframe()
    stl.SetColor((0.5,0.5,0.5))
    
    polydata = stl.src.GetOutput()
    s = cam.STLSurf()
    camvtk.vtkPolyData2OCLSTL(polydata, s)
    print "STL surface read ", s.size(), " triangles"
    
    cutter = cam.BallCutter(1.4321)
    
    #cutter = cam.CylCutter(1.123)
    
    #cutter = cam.BullCutter(1.123, 0.2)
    
    #cutter = cam.ConeCutter(0.43, math.pi/7)
    
    print cutter.str()
    
    #define grid of CL-points
    minx=-42
    dx=0.2
    maxx=47
    miny=-27
    dy=1
    maxy=20
    z=-55
    clpoints = CLPointGrid(minx,dx,maxx,miny,dy,maxy,z)
    print "generated grid with", len(clpoints)," CL-points"
    
    # batchdropcutter    
    bdc1 = cam.BatchDropCutter()
    bdc1.setSTL(s,1)
    bdc1.setCutter(cutter)
    
    # push the points to ocl
    for p in clpoints:
        bdc1.appendPoint(p)
   
    # run the actual calculation
    t_before = time.time()    
    bdc1.dropCutter4()
    t_after = time.time()
    calctime = t_after-t_before
    print " done in ", calctime," s"    

    
    # get back results from ocl
    cl1 = bdc1.getCLPoints()
    cc1 = bdc1.getCCPoints()
    
    # draw the results
    print "rendering...",
    #drawPoints(myscreen, cc1, cc1)
    drawPoints(myscreen, cl1, cc1)
    print "done"
    
    
    myscreen.camera.SetPosition(3, 23, 15)
    myscreen.camera.SetFocalPoint(4, 5, 0)
    
    
    # ocl text
    t = camvtk.Text()
    t.SetText("OpenCAMLib 10.04")
    t.SetPos( (myscreen.width-200, myscreen.height-30) )
    myscreen.addActor( t)
    
    # other text
    t2 = camvtk.Text()
    stltext = "%i triangles\n%i CL-points\n%0.1f seconds" % (s.size(), len(cl1), calctime)
    t2.SetText(stltext)
    t2.SetPos( (50, myscreen.height-100) )
    myscreen.addActor( t2)
    
    t3 = camvtk.Text()
    ctext = "Cutter: %s" % ( cutter.str() )
    
    t3.SetText(ctext)
    t3.SetPos( (50, myscreen.height-150) )
    myscreen.addActor( t3)
    
    myscreen.render()
    myscreen.iren.Start()
    raw_input("Press Enter to terminate") 
    
