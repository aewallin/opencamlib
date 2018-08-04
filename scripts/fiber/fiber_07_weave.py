import ocl
import camvtk
import time
import vtk
import datetime
import math

# note 2018.08:
# the Weave class is not provided to python, so this deprecated example script won't run

if __name__ == "__main__":  
    print ocl.version()
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
    
    cutter = ocl.CylCutter(0.3, 5)
    #cutter = ocl.BallCutter(0.4, 5)
    #cutter = ocl.BullCutter(0.4, 0.1, 5)
    
    print "fiber..."
    range=4
    Nmax = 100
    yvals = [float(n-float(Nmax)/2)/Nmax*range for n in xrange(0,Nmax+1)]
    xvals = [float(n-float(Nmax)/2)/Nmax*range for n in xrange(0,Nmax+1)]
    zmin = -0.1
    zmax = 0.5
    zNmax = 5
    dz = (zmax-zmin)/(zNmax-1)
    zvals=[ 0.23]
    #zvals.append(0.2)
    #for n in xrange(0,zNmax):
    #    zvals.append(zmin+n*dz)

    bpc_x = ocl.BatchPushCutter()
    bpc_y = ocl.BatchPushCutter()
    bpc_x.setXDirection()
    bpc_y.setYDirection()
    bpc_x.setSTL(s)
    bpc_y.setSTL(s)
    bpc_x.setCutter(cutter)
    bpc_y.setCutter(cutter)
    # create fibers
    for zh in zvals:
        for y in yvals:
            f1 = ocl.Point(-0.5,y,zh) # start point of fiber
            f2 = ocl.Point(1.5,y,zh)  # end point of fiber
            f =  ocl.Fiber( f1, f2)
            bpc_x.appendFiber(f)
        for x in xvals:
            f1 = ocl.Point(x,-0.5,zh) # start point of fiber
            f2 = ocl.Point(x,1.5,zh)  # end point of fiber
            f =  ocl.Fiber( f1, f2)
            bpc_y.appendFiber(f)
            
    # run
    bpc_x.run()
    bpc_y.run()
    clpoints = bpc_x.getCLPoints()
    clp2 = bpc_y.getCLPoints()
    clpoints+=clp2
    xfibers = bpc_x.getFibers()
    yfibers = bpc_y.getFibers()
    fibers = xfibers+yfibers
    print " got ",len(xfibers)," xfibers"
    print " got ",len(yfibers)," yfibers"
    print "rendering fibers and CL-points."
    w = ocl.Weave()
    print "push fibers to Weave...",
    for f in fibers:
        w.addFiber(f)
    print "done."
    print "Weave build()...",
    w.build()
    print "done"
    print "face_traverse..."
    w.face_traverse()
    print "done."
    w_clpts = w.getCLPoints()
    w_ipts = w.getIPoints()
    w_edges = w.getEdges()
    w_loop = w.getLoops()
    
    print " weave: got ", len(w_clpts)," CL-points and ", len(w_ipts)," internal points"
    
    print " got: ", len(w_edges), " edges"
    print " got: ", len(w_loop), " loop points"
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
    zoffset=0.1
    dzoffset = 0.0005
    for e in w_edges:
        p1 = e[0]
        p2 = e[1]
        myscreen.addActor( camvtk.Line( p1=( p1.x,p1.y,p1.z+zoffset+ne*dzoffset), p2=(p2.x,p2.y,p2.z+zoffset+ne*dzoffset) ) )
        ne = ne+1
        
    print "done."
    myscreen.camera.SetPosition(0.5, 3, 2)
    myscreen.camera.SetFocalPoint(0.5, 0.5, 0)
    camvtk.drawArrows(myscreen,center=(-0.5,-0.5,-0.5))
    camvtk.drawOCLtext(myscreen)
    myscreen.render()    
    myscreen.iren.Start()
    #raw_input("Press Enter to terminate") 
