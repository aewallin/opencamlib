import ocl
import camvtk
import time
import vtk
import datetime
import math

# 2018.08: Weave not wrapped..

def generateRange(zmin,zmax,zNmax):
    if zNmax>1:
        dz = (float(zmax)-float(zmin))/(zNmax-1)
    else:
        dz = 0
    zvals=[]
    for n in range(0,zNmax):
        zvals.append(zmin+n*dz)
    return zvals
    
def waterline(cutter, s, zh, tol = 0.1 ):
    bpc = ocl.BatchPushCutter()
    bpc.setSTL(s)
    bpc.setCutter(cutter)
    bounds = s.getBounds()
    xmin= bounds[0] - 2*cutter.getRadius()
    xmax= bounds[1] + 2*cutter.getRadius()
    ymin= bounds[2] - 2*cutter.getRadius()
    ymax= bounds[3] + 2*cutter.getRadius()
    Nx= int( (xmax-xmin)/tol )
    Ny= int( (ymax-ymin)/tol )
    xvals = generateRange(xmin,xmax,Nx)
    yvals = generateRange(ymin,ymax,Ny)
    for y in yvals:
        f1 = ocl.Point(xmin,y,zh) # start point of fiber
        f2 = ocl.Point(xmax,y,zh)  # end point of fiber
        f =  ocl.Fiber( f1, f2)
        bpc.appendFiber(f)
    for x in xvals:
        f1 = ocl.Point(x,ymin,zh) # start point of fiber
        f2 = ocl.Point(x,ymax,zh)  # end point of fiber
        f =  ocl.Fiber( f1, f2)
        bpc.appendFiber(f)
    bpc.run()
    fibers = bpc.getFibers() # get fibers
    w = ocl.Weave()
    for f in fibers:
        w.addFiber(f)

    print(" build()...",)
    w.build()
    print("done")
    print(" split()...",)
    subw = w.get_components()
    print("done. graph has", len(subw),"sub-weaves")
    m=0
    loops = []
    for sw in subw:
        print(m," face_traverse...",)
        t_before = time.time()
        sw.face_traverse()
        t_after = time.time()
        calctime = t_after-t_before
        print("done in ", calctime," s.")
        sw_loops = sw.getLoops()
        for lop in sw_loops:
            loops.append(lop)
        m=m+1
    return loops
    
if __name__ == "__main__":  
    print(ocl.version())
    myscreen = camvtk.VTKScreen()
    #stl = camvtk.STLSurf("../stl/demo.stl")
    stl = camvtk.STLSurf("../../stl/gnu_tux_mod.stl")
    myscreen.addActor(stl)
    stl.SetWireframe()
    #stl.SetSurface()
    stl.SetColor(camvtk.cyan)
    polydata = stl.src.GetOutput()
    s = ocl.STLSurf()
    camvtk.vtkPolyData2OCLSTL(polydata, s)
    print("STL surface read,", s.size(), "triangles")
    zh=1.9
    cutter_diams = generateRange(0.1, 6, 5)
    loops = []
    length = 20 # cutter length
    for diam in cutter_diams:
        cutter = ocl.CylCutter( diam, length )
        cutter_loops = waterline(cutter, s, zh, 0.05 )
        for l in cutter_loops:
            loops.append(l)

    print("All waterlines done. Got", len(loops)," loops in total.")
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

    print("done.")
    myscreen.camera.SetPosition(0.5, 3, 2)
    myscreen.camera.SetFocalPoint(0.5, 0.5, 0)
    camvtk.drawArrows(myscreen,center=(-0.5,-0.5,-0.5))
    camvtk.drawOCLtext(myscreen)
    myscreen.render()    
    myscreen.iren.Start()
    #raw_input("Press Enter to terminate") 
