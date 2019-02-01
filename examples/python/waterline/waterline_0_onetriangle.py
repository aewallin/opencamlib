import ocl
import camvtk
import time
import vtk
import datetime
import math

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
    zheights=[-0.3, -0.2, -0.1, -0.05, 0.0, 0.05, 0.1, 0.15, 0.2, 0.25, 0.26, 0.27, 0.28, 0.29 ]  # the z-coordinates for the waterlines
    zheights=[-0.8, -0.7, -0.6, -0.5, -0.4, -0.3, -0.2, -0.1, -0.05, 0.0, 0.05, 0.1, 0.15, 0.2,  0.28 ]
    zheights=[ -0.35, -0.3, -0.25, -0.2, -0.15, -0.1, -0.05, 0.0, 0.05, 0.1, 0.15, 0.2,  0.25, 0.28]
    cutter_diams = [0.6] # run the thing for all these cutter diameters
    length = 5
    loops = []
    cutter = ocl.CylCutter( 1 , 1 )   
    
    t_total = time.time() 
    for zh in zheights:
        for diam in cutter_diams:
            cutter = ocl.CylCutter( diam , length )
            cutter = ocl.BallCutter( diam , length )
            #cutter = ocl.BullCutter( diam , diam/5, length )
            #cutter = ocl.ConeCutter( diam , math.pi/5, length )
            wl = ocl.Waterline()
            #wl.setThreads(1)
            wl.setSTL(s)
            wl.setCutter(cutter)
            wl.setZ(zh)
            wl.setSampling(0.02)
            wl.setThreads(1)
            t_before = time.time() 
            wl.run()
            t_after = time.time()
            calctime = t_after-t_before
            print(" Waterline done in ", calctime," s")
            cutter_loops = wl.getLoops()
            for l in cutter_loops:
                loops.append(l)
    print(" ALL Waterlines done in ", time.time()-t_total," s")
    #print loops
    print("All waterlines done. Got", len(loops)," loops in total.")
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
    
    print("done.")
    myscreen.camera.SetPosition(2, 2, 2)
    myscreen.camera.SetFocalPoint(0.5, 0.5, 0)
    camvtk.drawArrows(myscreen,center=(-0.5,-0.5,-0.5))
    camvtk.drawOCLtext(myscreen)
    myscreen.render()    
    myscreen.iren.Start()
    #raw_input("Press Enter to terminate") 
