import ocl
import camvtk
import time
import vtk
import datetime
import math

def drawLoops(myscreen, loops, loopcolor):
    nloop = 0
    for lop in loops:
        n = 0
        N = len(lop)
        first_point=ocl.Point(-1,-1,5)
        previous=ocl.Point(-1,-1,5)
        for p in lop:
            if n==0: # don't draw anything on the first iteration
                previous=p 
                first_point = p
            elif n== (N-1): # the last point
                myscreen.addActor( camvtk.Line(p1=(previous.x,previous.y,previous.z),p2=(p.x,p.y,p.z),color=loopcolor) ) # the normal line
                # and a line from p to the first point
                myscreen.addActor( camvtk.Line(p1=(p.x,p.y,p.z),p2=(first_point.x,first_point.y,first_point.z),color=loopcolor) )
            else:
                myscreen.addActor( camvtk.Line(p1=(previous.x,previous.y,previous.z),p2=(p.x,p.y,p.z),color=loopcolor) )
                previous=p
            n=n+1
        print "rendered loop ",nloop, " with ", len(lop), " points"
        nloop = nloop+1
        
def getWaterline(s, cutter, zh, sampling):
    wl = ocl.Waterline()
    #wl.setThreads(1) # single thread for easier debug
    wl.setSTL(s)
    wl.setCutter(cutter)
    wl.setZ(zh)
    wl.setSampling(sampling)
    wl.run()
    loops = wl.getLoops()
    return loops

def getPaths(s,cutter,sampling,x,y):
    #apdc = ocl.PathDropCutter()
    apdc = ocl.AdaptivePathDropCutter()
    apdc.setSTL(s)
    apdc.setCutter(cutter) 
    apdc.setZ( -20 ) 
    apdc.setSampling(sampling)
    apdc.setMinSampling(sampling/700)
    path = ocl.Path() 
    p1 = ocl.Point(-x , y,-111)   # start-point of line
    p2 = ocl.Point(+x, y,-111)   # end-point of line
    #print p1
    #print p2
    l = ocl.Line(p1,p2)     # line-object
    path.append( l )  
    apdc.setPath( path )
    apdc.run() 
    print " PYTHON apdc.run() done."
    aclp = apdc.getCLPoints()
    print " PYTHON apdc.getCLPoints() done.", len(aclp)
    #for p in aclp:
    #    print p
    print " returning "
    return aclp

if __name__ == "__main__":  
    print ocl.revision()
    myscreen = camvtk.VTKScreen()
    #stl = camvtk.STLSurf("../stl/demo.stl")
    #stl = camvtk.STLSurf("../stl/30sphere.stl")
    #myscreen.addActor(stl)
    
    base=0.1
    tip=10
    a=ocl.Point(base,0,-tip)
    myscreen.addActor(camvtk.Point(center=(a.x,a.y,a.z), color=(1,0,1)));
    b=ocl.Point(-base,0,-tip)    
    myscreen.addActor(camvtk.Point(center=(b.x,b.y,b.z), color=(1,0,1)));
    c=ocl.Point(0,0,0)
    myscreen.addActor( camvtk.Point(center=(c.x,c.y,c.z), color=(1,0,1)));
    #myscreen.addActor( camvtk.Line(p1=(1,0,0),p2=(0,0,0.3)) )
    #myscreen.addActor( camvtk.Line(p1=(0,0,0.3),p2=(0,1,0)) )
    #myscreen.addActor( camvtk.Line(p1=(1,0,0),p2=(0,1,0)) )
    t = ocl.Triangle(a,b,c)
    s = ocl.STLSurf()
    s.addTriangle(t)
    
    print "STL surface read,", s.size(), "triangles"
    
    Nwaterlines = 10
    zh=[-0.15*x for x in xrange(Nwaterlines)]
    #zh=[15]
    diam = 3.0
    length = 50
    loops = []
    sampling = 0.1
    
    #cutter = ocl.CylCutter( diam , length )
    cutter = ocl.BallCutter( diam , length )
    #cutter = ocl.BullCutter( diam , diam/5, length )
    
    pts = []
    for y in [diam*0.4, diam*0.2,0,-diam*0.2,diam*(-0.4)]:
        ptsy = getPaths(s,cutter,sampling,1.52*cutter.getDiameter(), y)
        pts.append(ptsy)
        
    #print " got ",len(pts)," cl-points"
    #for p in pts:
    #    print p.x," ",p.y," ",p.z
    #exit()
    
    loops = []
    for z in zh:
        
        z_loops = getWaterline(s, cutter, z, sampling)
        for l in z_loops:
            loops.append(l)
    
    
    #for l in line:
        
    #drawLoops(myscreen, line, camvtk.cyan)
    #for l in cutter_loops:
    #    loops.append(l)
    
    print "All waterlines done. Got", len(loops)," loops in total."
    # draw the loops
    drawLoops(myscreen, loops, camvtk.yellow)
    drawLoops(myscreen, pts, camvtk.red)
    
    print "done."
    myscreen.camera.SetPosition(15, 13, 7)
    myscreen.camera.SetFocalPoint(5, 5, 0)
    camvtk.drawArrows(myscreen,center=(0,0,3))
    camvtk.drawOCLtext(myscreen)
    myscreen.render()    
    myscreen.iren.Start()
    #raw_input("Press Enter to terminate") 
