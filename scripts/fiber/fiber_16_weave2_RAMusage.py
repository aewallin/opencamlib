import ocl
import camvtk
import time
import vtk
import datetime
import math
import procmemory
import csv
import gc

# 2018.08: Weave not wrapped

def drawVertices(myscreen, weave, vertexType, vertexRadius, vertexColor):
    pts = weave.getVertices( vertexType )
    print " got ",len(pts)," of type ", vertexType
    for p in pts:
        myscreen.addActor( camvtk.Sphere(center=(p.x,p.y,p.z), radius=vertexRadius, color=vertexColor ) )

def getWeaveRAM(Nmax,weave2_flag):
    #stl = camvtk.STLSurf("../stl/demo.stl")
    stl = camvtk.STLSurf("../../stl/gnu_tux_mod.stl")
    polydata = stl.src.GetOutput()
    s = ocl.STLSurf()
    camvtk.vtkPolyData2OCLSTL(polydata, s)
    cutter = ocl.CylCutter(0.3, 5)
    fiber_range=30
    #Nmax = 400
    yvals = [float(n-float(Nmax)/2)/Nmax*float(fiber_range) for n in xrange(0,Nmax+1)]
    xvals = [float(n-float(Nmax)/2)/Nmax*float(fiber_range) for n in xrange(0,Nmax+1)]
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
    print " got ",len(xfibers)," xfibers"
    print " got ",len(yfibers)," yfibers"
    fibercount = len(xfibers) + len(yfibers)
    print "rendering fibers and CL-points."
    w = ocl.Weave()
    print "push fibers to Weave...",
    for f in fibers:
        w.addFiber(f)
    print "done."
    print "Weave build()...",
    mem1 = procmemory.memory()
    if weave2_flag==1:
        w.build2()
    else:
        w.build()
    mem2 = procmemory.memory()
    #print "after ", float(mem2)/float(1024*1024), " MB"
    megabytes = float(mem2-mem1)/float(1024*1024)
    megabytes2 = float(mem2)/float(1024*1024)
    verts = w.numVertices()
    print " before: ", float(mem1)/float(1024*1024)
    print "  after: ", float(mem2)/float(1024*1024)
    print " build() memory: ",megabytes," MB"
    print "done"
    print "face_traverse..."
    w.face_traverse()
    print "done."
    w_clpts = w.getCLVertices()
    w_ipts = w.getINTVertices()
    w_edges = w.getEdges()
    w_loop = w.getLoops()


    print " got: ", len(w_edges), " edges"
    print " got: ", len(w_loop), " loops"
    out=[]
    out.append(fibercount)
    out.append(verts)
    return out
    
if __name__ == "__main__":  
    writer = csv.writer( open('weave2ram_6.csv','wb'), delimiter=',')
    
    #Nmax=400
    Nmaxlist = [100,200,400,800, 1600, 3200]

    for Nmax in Nmaxlist:
        mb = getWeaveRAM(Nmax,0)
        mb2 = getWeaveRAM(Nmax,1)
        #print mb2
        print mb[0], " :=> ", mb[1], " MB"
        writer.writerow(mb+mb2)
        gc.collect()

    raw_input("Press Enter to terminate") 
