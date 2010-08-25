import ocl
import camvtk
import time
import vtk
import datetime
import math
def drawFiber(myscreen, f, fibercolor=camvtk.red):
    inter = f.getInts()
    for i in inter:
        if not i.empty():
            ip1 = f.point( i.lower )
            ip2 = f.point( i.upper )
            myscreen.addActor( camvtk.Line(p1=(ip1.x,ip1.y,ip1.z),p2=(ip2.x,ip2.y,ip2.z), color=fibercolor) )
            myscreen.addActor( camvtk.Sphere(center=(ip1.x,ip1.y,ip1.z),radius=0.005, color=camvtk.clColor( i.lower_cc) ) )
            myscreen.addActor( camvtk.Sphere(center=(ip2.x,ip2.y,ip2.z),radius=0.005, color=camvtk.clColor( i.upper_cc) ) )
            cc1 = i.lower_cc
            cc2 = i.upper_cc
            myscreen.addActor( camvtk.Sphere(center=(cc1.x,cc1.y,cc1.z),radius=0.005, color=camvtk.lgreen ) )
            myscreen.addActor( camvtk.Sphere(center=(cc2.x,cc2.y,cc2.z),radius=0.005, color=camvtk.lgreen ) )
            
            #if cc1.type == ocl.CCType.FACET:
            #    cactor = camvtk.Sphere(center=(ip1.x,ip1.y,ip1.z+cutter.radius), radius=cutter.radius, color=camvtk.cyan ) 
            #    cactor.SetOpacity(0.2)
            #    myscreen.addActor( cactor )
        
            
            # cutter circle
            #c1 = camvtk.Circle(center=(ip1.x,ip1.y,ip1.z), radius = 0.3/2, color=fibercolor)
            #myscreen.addActor(c1)
            #c2 = camvtk.Circle(center=(ip2.x,ip2.y,ip2.z), radius = 0.3/2, color=fibercolor)
            #myscreen.addActor(c2)
            
if __name__ == "__main__":  
    print ocl.revision()
    
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
    
    #cutter = ocl.CylCutter(0.3)
    cutter = ocl.BallCutter(0.4)
    #cutter = ocl.BullCutter(0.4, 0.1)
    
    cutter.length = 4.0
    print "lengt=", cutter.length
    print "radius=", cutter.radius
    print "fiber..."
    range=4
    Nmax = 100
    yvals = [float(n-float(Nmax)/2)/Nmax*range for n in xrange(0,Nmax+1)]
    xvals = [float(n-float(Nmax)/2)/Nmax*range for n in xrange(0,Nmax+1)]
    zmin = -0.1
    zmax = 0.5
    zNmax = 5
    dz = (zmax-zmin)/(zNmax-1)
    zvals=[ 0.15]
    #zvals.append(0.2)
    #for n in xrange(0,zNmax):
    #    zvals.append(zmin+n*dz)

    bpc = ocl.BatchPushCutter()
    bpc.setSTL(s)
    bpc.setCutter(cutter)
    # create fibers
    for zh in zvals:
        for y in yvals:
            f1 = ocl.Point(-0.5,y,zh) # start point of fiber
            f2 = ocl.Point(1.5,y,zh)  # end point of fiber
            f =  ocl.Fiber( f1, f2)
            bpc.appendFiber(f)
        for x in xvals:
            f1 = ocl.Point(x,-0.5,zh) # start point of fiber
            f2 = ocl.Point(x,1.5,zh)  # end point of fiber
            f =  ocl.Fiber( f1, f2)
            bpc.appendFiber(f)
            
    # run
    bpc.pushCutter3()
    clpoints = bpc.getCLPoints()
    fibers = bpc.getFibers()
    print " got ",len(fibers)," fibers from bpc"
    print "rendering fibers and CL-points."
    #camvtk.drawCLPointCloud(myscreen, clpoints)
    for f in fibers:
        drawFiber(myscreen, f, camvtk.red)
    
    #exit()
    
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
    """
    ne = 0
    zoffset=0.2
    dzoffset = 0
    for e in w_edges:
        p1 = e[0]
        p2 = e[1]
        myscreen.addActor( camvtk.Line( p1=( p1.x,p1.y,p1.z+zoffset+ne*dzoffset), p2=(p2.x,p2.y,p2.z+zoffset+ne*dzoffset) ) )
        ne = ne+1
    """
        
    print "done."
    myscreen.camera.SetPosition(0.5, 3, 2)
    myscreen.camera.SetFocalPoint(0.5, 0.5, 0)
    camvtk.drawArrows(myscreen,center=(-0.5,-0.5,-0.5))
    camvtk.drawOCLtext(myscreen)

    myscreen.render()    

    myscreen.iren.Start()
    #raw_input("Press Enter to terminate") 
