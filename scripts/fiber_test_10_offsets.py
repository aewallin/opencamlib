import ocl
import camvtk
import time
import vtk
import datetime
import math

def generateRange(zmin,zmax,zNmax):
    dz = (float(zmax)-float(zmin))/(zNmax-1)
    zvals=[]
    for n in xrange(0,zNmax):
        zvals.append(zmin+n*dz)
    return zvals
    
def waterline(cutter, s, zh, xvals, yvals ):
    bpc = ocl.BatchPushCutter()
    bpc.setSTL(s)
    bpc.setCutter(cutter)
    # create fibers
    nfibers=0
    for y in yvals:
        f1 = ocl.Point(xmin,y,zh) # start point of fiber
        f2 = ocl.Point(xmax,y,zh)  # end point of fiber
        f =  ocl.Fiber( f1, f2)
        bpc.appendFiber(f)
        nfibers=nfibers+1
    for x in xvals:
        f1 = ocl.Point(x,ymin,zh) # start point of fiber
        f2 = ocl.Point(x,ymax,zh)  # end point of fiber
        f =  ocl.Fiber( f1, f2)
        bpc.appendFiber(f)
        nfibers=nfibers+1
    bpc.pushCutter3()
    fibers = bpc.getFibers() # get fibers
    w = ocl.Weave()
    for f in fibers:
        w.addFiber(f)

    print " build()...",
    w.build()
    print "done"
    print " split()...",
    subw = w.get_components()
    print " graph has ", len(subw)," sub-weaves"
    m=0
    loops = []
    for sw in subw:
        print m," face_traverse...",
        t_before = time.time()
        sw.face_traverse()
        t_after = time.time()
        calctime = t_after-t_before
        print "done in ", calctime," s"
        sw_loops = sw.getLoops()
        for lop in sw_loops:
            loops.append(lop)
        m=m+1
    return loops
    
if __name__ == "__main__":  
    print ocl.revision()
    myscreen = camvtk.VTKScreen()
    #stl = camvtk.STLSurf("../stl/demo.stl")
    stl = camvtk.STLSurf("../stl/gnu_tux_mod.stl")
    myscreen.addActor(stl)
    stl.SetWireframe()
    #stl.SetSurface()
    stl.SetColor(camvtk.cyan)
    polydata = stl.src.GetOutput()
    s = ocl.STLSurf()
    camvtk.vtkPolyData2OCLSTL(polydata, s)
    print "STL surface read,", s.size(), "triangles"
    bounds = s.getBounds()
    print "STLSurf.Bounds()=", bounds
    cutter = ocl.CylCutter(0.1)
    #cutter = ocl.BallCutter(0.3)
    cutter.length = 4.0
    print cutter
    xmin=-1
    xmax=15
    N=100
    ymin=-1
    ymax=15
    yvals = generateRange(ymin,ymax,N)
    xvals = generateRange(xmin,xmax,N)
    #print xvals
    zmin = -0.1
    zmax = 2.75
    zNmax = 5
    zh=1.9
    cutter_diams = generateRange(0.1, 6, 10)
    loops = []
    for diam in cutter_diams:
        cutter = ocl.CylCutter( diam )
        cutter_loops = waterline(cutter, s, zh, xvals, yvals)
        for l in cutter_loops:
            loops.append(l)

    print " got", len(loops)," loops"
    # draw the loops
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

    print "done."
    myscreen.camera.SetPosition(0.5, 3, 2)
    myscreen.camera.SetFocalPoint(0.5, 0.5, 0)
    camvtk.drawArrows(myscreen,center=(-0.5,-0.5,-0.5))
    camvtk.drawOCLtext(myscreen)

    myscreen.render()    

    myscreen.iren.Start()
    #raw_input("Press Enter to terminate") 
