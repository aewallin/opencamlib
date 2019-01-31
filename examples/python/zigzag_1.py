import ocl
import camvtk
import time
import vtk
import datetime
import math

def DrawLoops(myscreen, loops):
    # draw the loops
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
                myscreen.addActor( camvtk.Line(p1=(previous.x,previous.y,previous.z),p2=(p.x,p.y,p.z),color=camvtk.yellow) ) # the normal line
                # and a line from p to the first point
                myscreen.addActor( camvtk.Line(p1=(p.x,p.y,p.z),p2=(first_point.x,first_point.y,first_point.z),color=camvtk.yellow) )
            else:
                myscreen.addActor( camvtk.Line(p1=(previous.x,previous.y,previous.z),p2=(p.x,p.y,p.z),color=camvtk.yellow) )
                previous=p
            n=n+1
        print("rendered loop ",nloop, " with ", len(lop), " points")
        nloop = nloop+1

def drawOutput(myscreen, pts, stepOver):
    for p in pts:
        myscreen.addActor( camvtk.Sphere(center=(p.x,p.y,p.z), radius=stepOver/5, color=camvtk.red) )

if __name__ == "__main__":  
    print(ocl.version())
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
    zheight=0.15  # the z-coordinate for the waterline
    diam = 0.6 
    length = 5
    loops = []
    #cutter = ocl.CylCutter( 1 , 1 )   

    cutter = ocl.CylCutter( diam , length )
    #cutter = ocl.BallCutter( diam , length )
    #cutter = ocl.BullCutter( diam , diam/5, length )
    wl = ocl.Waterline()
    #wl.setThreads(1)
    wl.setSTL(s)
    wl.setCutter(cutter)
    wl.setZ(zheight)
    wl.setSampling(0.1)
    t_before = time.time() 
    wl.run()
    t_after = time.time()
    calctime = t_after-t_before
    print(" Waterline done in ", calctime," s")
    cutter_loops = wl.getLoops()
    for l in cutter_loops:
        loops.append(l)
    #print loops
    print("All waterlines done. Got", len(loops)," loops in total.")
    DrawLoops(myscreen, loops)
    
    z = ocl.ZigZag()
    z.setOrigin( ocl.Point(0,0) )
    z.setDirection( ocl.Point(1,1) )
    stepOver = 0.021
    z.setStepOver( stepOver )
    for lop in loops:
        for p in lop:
            z.addPoint(p)
    z.run()
    print(z)
    out = z.getOutput()
    print("got ",len(out)," output points")
    drawOutput(myscreen, out, stepOver )
    
    print("done.")
    myscreen.camera.SetPosition(2, 2, 2)
    myscreen.camera.SetFocalPoint(0.5, 0.5, 0)
    camvtk.drawArrows(myscreen,center=(-0.5,-0.5,-0.5))
    camvtk.drawOCLtext(myscreen)
    myscreen.render()    
    myscreen.iren.Start()
    #raw_input("Press Enter to terminate") 
