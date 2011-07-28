import ocl
import camvtk
import time
import vtk
import datetime
import math

def calcWaterline(zh, cutter,s):
    wl = ocl.Waterline()
    wl.setSTL(s)
    wl.setCutter(cutter)
    wl.setZ(zh)
    wl.setSampling(0.04)
    wl.setThreads(1)
    t_before = time.time() 
    wl.run()
    t_after = time.time()
    calctime = t_after-t_before
    print " Waterline done in ", calctime," s"
    return wl.getLoops()


def drawLoops(myscreen, loops, loopColor=camvtk.yellow):
    nloop=0
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
                myscreen.addActor( camvtk.Line(p1=(previous.x,previous.y,previous.z),p2=(p.x,p.y,p.z),color=loopColor) ) # the normal line
                # and a line from p to the first point
                myscreen.addActor( camvtk.Line(p1=(p.x,p.y,p.z),p2=(first_point.x,first_point.y,first_point.z),color=loopColor) )
            else:
                myscreen.addActor( camvtk.Line(p1=(previous.x,previous.y,previous.z),p2=(p.x,p.y,p.z),color=loopColor) )
                previous=p
            n=n+1
        print "rendered loop ",nloop, " with ", len(lop), " points"
        nloop = nloop+1

if __name__ == "__main__":  
    print ocl.revision()
    myscreen = camvtk.VTKScreen()
    a = ocl.Point(0,1,0.3)
    myscreen.addActor(camvtk.Point(center=(a.x,a.y,a.z), color=(1,0,1)))
    b = ocl.Point(1,0.5,0.3)    
    myscreen.addActor(camvtk.Point(center=(b.x,b.y,b.z), color=(1,0,1)))
    c = ocl.Point(0,0,0)
    myscreen.addActor(camvtk.Point(center=(c.x,c.y,c.z), color=(1,0,1)))
    myscreen.addActor( camvtk.Line(p1=(a.x,a.y,a.z),p2=(c.x,c.y,c.z)) )
    myscreen.addActor( camvtk.Line(p1=(c.x,c.y,c.z),p2=(b.x,b.y,b.z)) )
    myscreen.addActor( camvtk.Line(p1=(a.x,a.y,a.z),p2=(b.x,b.y,b.z)) )
    t = ocl.Triangle(b,c,a)
    s = ocl.STLSurf()
    s.addTriangle(t) # a one-triangle STLSurf
    zheights=[-0.3, -0.2, -0.1, -0.05, 0.0, 0.05, 0.1, 0.15, 0.2, 0.25, 0.26, 0.27, 0.28, 0.29 ]  # the z-coordinates for the waterlines
    zheights=[-0.8, -0.7, -0.6, -0.5, -0.4, -0.3, -0.2, -0.1, -0.05, 0.0, 0.05, 0.1, 0.15, 0.2,  0.28 ]
    zheights=[ -0.35, -0.3, -0.25, -0.2, -0.15, -0.1, -0.05, 0.0, 0.05, 0.1, 0.15, 0.2,  0.25, 0.28]
    
    zheights=[]
    Nmax=30
    zmin=-0.45
    zmax= 0.28
    dz = (zmax-zmin)/float(Nmax-1)
    z = zmin
    for n in xrange(Nmax):
        zheights.append(z)
        z=z+dz
    #zheights=[ -0.35,  -0.25,  -0.15,  -0.05, 0.05,  0.15,   0.25]
    #zheights=[ 0.1]
    
    length = 20
    diam = 0.6

    for zh in zheights:
        cutter1 = ocl.CylCutter( diam , length )
        cutter2 = ocl.BallCutter( diam , length )
        cutter3 = ocl.BullCutter( diam , diam/5, length )
        cutter4 = ocl.ConeCutter( diam , math.pi/5, length )
        
        #loops1 = calcWaterline(zh, cutter1, s)
        #drawLoops(myscreen, loops1, camvtk.red)
        
        #loops2 = calcWaterline(zh, cutter2, s)
        #drawLoops(myscreen, loops2, camvtk.green)
        #loops3 = calcWaterline(zh, cutter3, s)
        #drawLoops(myscreen, loops3, camvtk.yellow)
        loops4 = calcWaterline(zh, cutter4, s)
        drawLoops(myscreen, loops4, camvtk.pink)
        
        
        

    print "done."
    myscreen.camera.SetPosition(2, 2, 2)
    myscreen.camera.SetFocalPoint(0.5, 0.5, 0)
    camvtk.drawArrows(myscreen,center=(-0.5,-0.5,-0.5))
    camvtk.drawOCLtext(myscreen)
    myscreen.render()    
    myscreen.iren.Start()
    #raw_input("Press Enter to terminate") 
