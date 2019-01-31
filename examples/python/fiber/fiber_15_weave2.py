import ocl
import camvtk
import time
import vtk
import datetime
import math

# 2018.08: Weave not wrapped

def drawVertices(myscreen, weave, vertexType, vertexRadius, vertexColor):
    pts = weave.getVertices( vertexType )
    print(" got ",len(pts)," of type ", vertexType)
    for p in pts:
        myscreen.addActor( camvtk.Sphere(center=(p.x,p.y,p.z), radius=vertexRadius, color=vertexColor ) )

if __name__ == "__main__":  
    print(ocl.version())
    myscreen = camvtk.VTKScreen()
    a = ocl.Point(0,1,0.3)
    #myscreen.addActor(camvtk.Point(center=(a.x,a.y,a.z), color=(1,0,1)))
    b = ocl.Point(1,0.5,0.3)    
    #myscreen.addActor(camvtk.Point(center=(b.x,b.y,b.z), color=(1,0,1)))
    c = ocl.Point(0,0,0.1)
    #myscreen.addActor(camvtk.Point(center=(c.x,c.y,c.z), color=(1,0,1)))
    #myscreen.addActor( camvtk.Line(p1=(a.x,a.y,a.z),p2=(c.x,c.y,c.z)) )
    #myscreen.addActor( camvtk.Line(p1=(c.x,c.y,c.z),p2=(b.x,b.y,b.z)) )
    #myscreen.addActor( camvtk.Line(p1=(a.x,a.y,a.z),p2=(b.x,b.y,b.z)) )
    
    # a second triangle
    offset=ocl.Point(0.51,0.51,0)
    a2 = a+ offset
    b2 = b+ offset    
    c2 = c+ offset
    t2 = ocl.Triangle(b2,c2,a2)
    
    t = ocl.Triangle(b,c,a)
    s = ocl.STLSurf()
    s.addTriangle(t) # a one-triangle STLSurf
    s.addTriangle(t2)
    
    cutter = ocl.CylCutter(0.3, 5)
    #cutter = ocl.BallCutter(0.4, 5)
    #cutter = ocl.BullCutter(0.4, 0.1, 5)
    
    print("fiber...")
    fiber_range=7
    Nmax = 200
    yvals = [float(n-float(Nmax)/2)/Nmax*float(fiber_range) for n in range(0,Nmax+1)]
    xvals = [float(n-float(Nmax)/2)/Nmax*float(fiber_range) for n in range(0,Nmax+1)]

    zvals=[ 0.23]


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
            f1 = ocl.Point(-5.5,y,zh) # start point of fiber
            f2 = ocl.Point(5.5,y,zh)  # end point of fiber
            f =  ocl.Fiber( f1, f2)
            bpc_x.appendFiber(f)
        for x in xvals:
            f1 = ocl.Point(x,-5.5,zh) # start point of fiber
            f2 = ocl.Point(x,5.5,zh)  # end point of fiber
            f =  ocl.Fiber( f1, f2)
            bpc_y.appendFiber(f)
            
    # run
    bpc_x.run()
    bpc_y.run()
    #clpoints = bpc_x.getCLPoints()
    #clp2 = bpc_y.getCLPoints()
    #clpoints+=clp2
    xfibers = bpc_x.getFibers()
    yfibers = bpc_y.getFibers()
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
    #w.build()
    w.build()
    print("done")
    print("face_traverse...")
    w.face_traverse()
    print("done.")
    w_clpts = w.getCLVertices()
    w_ipts = w.getINTVertices()
    w_edges = w.getEdges()
    w_loop = w.getLoops()
    vertexRadius = 0.007
    drawVertices(myscreen, w, ocl.WeaveVertexType.CL, vertexRadius, camvtk.red)
    drawVertices(myscreen, w, ocl.WeaveVertexType.INT, vertexRadius, camvtk.orange)
    drawVertices(myscreen, w, ocl.WeaveVertexType.FULLINT, vertexRadius, camvtk.yellow)
    drawVertices(myscreen, w, ocl.WeaveVertexType.ADJ, vertexRadius, camvtk.green)
    drawVertices(myscreen, w, ocl.WeaveVertexType.TWOADJ, vertexRadius, camvtk.lblue)

    print(" got: ", len(w_edges), " edges")
    print(" got: ", len(w_loop), " loop points")


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
    zoffset=0.0 # 1
    dzoffset = 0.000 # 5
    for e in w_edges:
        p1 = e[0]
        p2 = e[1]
        myscreen.addActor( camvtk.Line( p1=( p1.x,p1.y,p1.z+zoffset+ne*dzoffset), p2=(p2.x,p2.y,p2.z+zoffset+ne*dzoffset) ) )
        ne = ne+1
        
    print("done.")
    myscreen.camera.SetPosition(0.8051, 0.8051, 3.5)
    myscreen.camera.SetFocalPoint(0.805, 0.805, 0)
    camvtk.drawArrows(myscreen,center=(-0.5,-0.5,-0.5))
    camvtk.drawOCLtext(myscreen)
    myscreen.render()    
    myscreen.iren.Start()
    #raw_input("Press Enter to terminate") 
