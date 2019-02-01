import ocl
import camvtk
import time
import vtk
import datetime
import math

if __name__ == "__main__":  
    print(ocl.revision())
    myscreen = camvtk.VTKScreen()
    a = ocl.Point(0,1,0.3)
    myscreen.addActor(camvtk.Point(center=(a.x,a.y,a.z), color=(1,0,1)))
    b = ocl.Point(1,0.5,0.3)    
    myscreen.addActor(camvtk.Point(center=(b.x,b.y,b.z), color=(1,0,1)))
    c = ocl.Point(0,0,0.1)
    myscreen.addActor(camvtk.Point(center=(c.x,c.y,c.z), color=(1,0,1)))
    myscreen.addActor( camvtk.Line(p1=(a.x,a.y,a.z),p2=(c.x,c.y,c.z)) )
    myscreen.addActor( camvtk.Line(p1=(c.x,c.y,c.z),p2=(b.x,b.y,b.z)) )
    myscreen.addActor( camvtk.Line(p1=(a.x,a.y,a.z),p2=(b.x,b.y,b.z)) )
    t = ocl.Triangle(b,c,a)
    s = ocl.STLSurf()
    s.addTriangle(t) # a one-triangle STLSurf
    
    #cutter = ocl.CylCutter(0.31, 5)
    #cutter = ocl.BallCutter(0.4, 5)
    cutter = ocl.BullCutter(0.4, 0.1, 5)
    
    print("fiber...")

    zh =  0.23
    
    aloops = []
    awl = ocl.AdaptiveWaterline()
    #awl = ocl.Waterline()
    awl.setSTL(s)
    awl.setCutter(cutter)
    awl.setZ(zh)
    sampling=0.1
    awl.setSampling(sampling)
    awl.setMinSampling(0.0001)
    t_before = time.time() 
    awl.run()
    t_after = time.time()
    calctime = t_after-t_before
    print(" AdaptiveWaterline done in ", calctime," s")
    xfibers = awl.getXFibers()
    #print " got ", len(xf)," x-fibers"
    yfibers = awl.getYFibers()
    #print " got ", len(yf)," y-fibers"
    
    
    fibers = xfibers+yfibers
    print(" got ",len(xfibers)," xfibers")
    print(" got ",len(yfibers)," yfibers")
    print("rendering fibers and CL-points.")
    w = ocl.Weave()
    print("push fibers to Weave...",)
    for f in fibers:
        w.addFiber(f)
    print("done.")
    print("Weave build()...",)
    w.build()
    print("done")
    print("face_traverse...")
    w.face_traverse()
    print("done.")
    w_clpts = w.getCLPoints()
    w_ipts = w.getIPoints()
    w_edges = w.getEdges()
    w_loop = w.getLoops()
    
    print(" weave: got ", len(w_clpts)," CL-points and ", len(w_ipts)," internal points")
    
    print(" got: ", len(w_edges), " edges")
    print(" got: ", len(w_loop), " loop points")
    #zoffset = 0.0
    #for p in w_clpts:
    #    myscreen.addActor( camvtk.Sphere(center=(p.x,p.y,p.z+zoffset), radius=0.0031, color=camvtk.red ) )

    previous = 0
    # draw the loop as a yellow line
    for loop in w_loop:
        np = 0
        for p in loop:
            #myscreen.addActor( camvtk.Sphere(center=(p.x,p.y,p.z+zoffset2), radius=0.006, color=camvtk.pink ) )        
            if np is not 0:
                myscreen.addActor( camvtk.Line( p1=(previous.x,previous.y, previous.z), 
                                                p2=(p.x,p.y,p.z), color=camvtk.yellow) )
            np=np+1
            previous = p
    
    # draw edges of weave
    ne = 0
    zoffset=0.0
 
    dzoffset =  0.001
    dzoffset2 = 0.000
    for e in w_edges:
        p1 = e[0]
        p2 = e[1]
        df = p1-p2
        if (df.norm() > 1.05*sampling ):
            myscreen.addActor( camvtk.Line( p1=( p1.x,p1.y,p1.z+zoffset+ne*dzoffset), p2=(p2.x,p2.y,p2.z+zoffset+ne*dzoffset) ) )
        else:
            myscreen.addActor( camvtk.Line( p1=( p1.x,p1.y,p1.z+zoffset+ne*dzoffset2), p2=(p2.x,p2.y,p2.z+zoffset+ne*dzoffset2) ) )
        ne = ne+1
        
    print("done.")
    myscreen.camera.SetPosition(0.5, 3, 2)
    myscreen.camera.SetFocalPoint(0.5, 0.5, 0)
    camvtk.drawArrows(myscreen,center=(-0.5,-0.5,-0.5))
    camvtk.drawOCLtext(myscreen)
    myscreen.render()    
    myscreen.iren.Start()
    #raw_input("Press Enter to terminate") 
