import ocl
import camvtk
import time
import vtk
import datetime
import math
import procmemory

# 2018.08: Weave not wrapped

def drawVertices(myscreen, weave, vertexType, vertexRadius, vertexColor):
    pts = weave.getVertices( vertexType )
    print(" got ",len(pts)," of type ", vertexType)
    for p in pts:
        myscreen.addActor( camvtk.Sphere(center=(p.x,p.y,p.z), radius=vertexRadius, color=vertexColor ) )

if __name__ == "__main__":  
    print(ocl.version())
    myscreen = camvtk.VTKScreen()
    #stl = camvtk.STLSurf("../stl/demo.stl")
    stl = camvtk.STLSurf("../../stl/gnu_tux_mod.stl")
    myscreen.addActor(stl)
    stl.SetWireframe()
    #stl.SetSurface()
    stl.SetColor(camvtk.grey)
    polydata = stl.src.GetOutput()
    s = ocl.STLSurf()
    camvtk.vtkPolyData2OCLSTL(polydata, s)
    
    cutter = ocl.CylCutter(0.3, 5)
    #cutter = ocl.BallCutter(0.4, 5)
    #cutter = ocl.BullCutter(0.4, 0.1, 5)
    
    print("fiber...")
    fiber_range=30
    Nmax = 400
    yvals = [float(n-float(Nmax)/2)/Nmax*float(fiber_range) for n in range(0,Nmax+1)]
    xvals = [float(n-float(Nmax)/2)/Nmax*float(fiber_range) for n in range(0,Nmax+1)]

    zvals=[ 1.6523]


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
            f1 = ocl.Point(-15.5,y,zh) # start point of fiber
            f2 = ocl.Point(15.5,y,zh)  # end point of fiber
            f =  ocl.Fiber( f1, f2)
            bpc_x.appendFiber(f)
        for x in xvals:
            f1 = ocl.Point(x,-15.5,zh) # start point of fiber
            f2 = ocl.Point(x,15.5,zh)  # end point of fiber
            f =  ocl.Fiber( f1, f2)
            bpc_y.appendFiber(f)
            
    # run
    bpc_x.run()
    bpc_y.run()

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
    mem1 = procmemory.resident()
    print("before ", mem1)
    w.build()
    #w.build2()
    mem2 = procmemory.resident()
    print("after ", float(mem2)/float(1024*1024), " MB")
    print(" build() memory: ",float(mem2-mem1)/float(1024*1024)," MB")
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
    print(" got: ", len(w_loop), " loops")
    
    # draw the loops
        
    nloop = 0
    for lop in w_loop:
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
