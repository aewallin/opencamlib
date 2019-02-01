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
        print("rendered loop ",nloop, " with ", len(lop), " points")
        nloop = nloop+1
        

def drawFibers(myscreen, fibs, fibcolor):
    for f in fibs:
    #print f
        for i in f.getInts():
            #print i
            p1 = f.point(i.lower)
            p2 = f.point(i.upper)
            #print p1
            #print p2
            myscreen.addActor( camvtk.Line(p1=(p1.x,p1.y,p1.z),p2=(p2.x,p2.y,p2.z),color=fibcolor) )
            
if __name__ == "__main__":  
    print(ocl.version())
    myscreen = camvtk.VTKScreen()
    a = ocl.Point(0,1,0.3)
    myscreen.addActor(camvtk.Point(center=(a.x,a.y,a.z), color=(1,0,1)))
    b = ocl.Point(1,1,0.3)    
    myscreen.addActor(camvtk.Point(center=(b.x,b.y,b.z), color=(1,0,1)))
    c = ocl.Point(0,0,0.1)
    c = ocl.Point(0,0,-2.1)
    #c = ocl.Point(0.5,0.5,-10)
    myscreen.addActor(camvtk.Point(center=(c.x,c.y,c.z), color=(1,0,1)))
    myscreen.addActor( camvtk.Line(p1=(a.x,a.y,a.z),p2=(c.x,c.y,c.z)) )
    myscreen.addActor( camvtk.Line(p1=(c.x,c.y,c.z),p2=(b.x,b.y,b.z)) )
    myscreen.addActor( camvtk.Line(p1=(a.x,a.y,a.z),p2=(b.x,b.y,b.z)) )
    t = ocl.Triangle(b,c,a)
    s = ocl.STLSurf()
    s.addTriangle(t) # a one-triangle STLSurf
    zh=0.25  # the z-coordinates for the waterlines
    diam = 0.31 # run the thing for all these cutter diameters
    length = 5
    loops = []

    #cutter = ocl.CylCutter( diam , length )
    #cutter = ocl.BallCutter( diam , length )
    #cutter = ocl.BullCutter( diam , diam/5, length )
    cutter = ocl.ConeCutter( diam , math.pi/4, length )
    
    wl = ocl.Waterline()
    #wl.setThreads(1)
    wl.setSTL(s)
    wl.setCutter(cutter)
    wl.setZ(zh)
    wl.setSampling(0.05)
    t_before = time.time() 
    wl.run()
    t_after = time.time()
    calctime = t_after-t_before
    print(" Waterline done in ", calctime," s")
    cutter_loops = wl.getLoops()
    for l in cutter_loops:
        loops.append(l)
    
    aloops = []
    awl = ocl.AdaptiveWaterline()
    awl.setSTL(s)
    awl.setCutter(cutter)
    awl.setZ(zh)
    awl.setSampling(0.05)
    awl.setMinSampling(0.0001)
    t_before = time.time() 
    awl.run()
    
    t_after = time.time()
    calctime = t_after-t_before
    print(" AdaptiveWaterline done in ", calctime," s")
    
    xf = awl.getXFibers()
    print(" got ", len(xf)," x-fibers")
    yf = awl.getYFibers()
    print(" got ", len(yf)," y-fibers")
    drawFibers(myscreen, xf, camvtk.red)
    drawFibers(myscreen, yf, camvtk.green)
    
    acutter_loops = awl.getLoops()
    #acutter_loops = []
    for l in acutter_loops:
        l2=[]
        zofs = 0.0
        for p in l:
            p2 = p + ocl.Point(0,0,zofs)
            l2.append(p2)
        aloops.append(l2)
    
    drawLoops(myscreen, loops, camvtk.yellow)
    drawLoops(myscreen, aloops, camvtk.mag)
    
    print("done.")
    myscreen.camera.SetPosition(2, 2, 2)
    myscreen.camera.SetFocalPoint(0.5, 0.5, 0)
    camvtk.drawArrows(myscreen,center=(-0.5,-0.5,-0.5))
    camvtk.drawOCLtext(myscreen)
    myscreen.render()    
    myscreen.iren.Start()
    #raw_input("Press Enter to terminate") 
