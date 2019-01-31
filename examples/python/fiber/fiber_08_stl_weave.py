import ocl
import camvtk
import time
import vtk
import datetime
import math

# note 2018.08: won't run becayse we don't have ocl.Weave()

def generateRange(zmin,zmax,zNmax):
    dz = (float(zmax)-float(zmin))/(zNmax-1)
    zvals=[]
    for n in range(0,zNmax):
        zvals.append(zmin+n*dz)
    return zvals
    

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
    bounds = s.getBounds()
    print("STLSurf.Bounds()=", bounds)
    cutter = ocl.CylCutter(1.6,5)
    #cutter = ocl.BallCutter(0.3,5)
        
    print(cutter)
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
    zvals = generateRange(zmin,zmax,zNmax)
    print(" calculating waterlines at ", len(zvals)," different z-heights")
    #print zvals
    bpcx = ocl.BatchPushCutter()
    bpcy = ocl.BatchPushCutter()
    bpcx.setXDirection()
    bpcy.setYDirection()
    bpcx.setSTL(s)
    bpcy.setSTL(s)
    bpcx.setCutter(cutter)
    bpcy.setCutter(cutter)
    # create fibers
    nfibers=0
    for zh in zvals:
        for y in yvals:
            f1 = ocl.Point(xmin,y,zh) # start point of fiber
            f2 = ocl.Point(xmax,y,zh)  # end point of fiber
            f =  ocl.Fiber( f1, f2)
            bpcx.appendFiber(f)
            nfibers=nfibers+1
        for x in xvals:
            f1 = ocl.Point(x,ymin,zh) # start point of fiber
            f2 = ocl.Point(x,ymax,zh)  # end point of fiber
            f =  ocl.Fiber( f1, f2)
            bpcy.appendFiber(f)
            nfibers=nfibers+1
            
    # run
    t_before = time.time() 
    bpcx.run()
    bpcy.run()
    t_after = time.time()
    push_calctime = t_after-t_before


    
    clpoints = bpcx.getCLPoints()
    clp2=bpcy.getCLPoints()
    clpoints+=clp2
    print("got ", len(clpoints), " CL-points")
    print("rendering raw CL-points.")
    
    # draw the CL-points
    #camvtk.drawCLPointCloud(myscreen, clpoints)
    #for p in clpoints:
    #     myscreen.addActor( camvtk.Sphere(center=(p.x,p.y,p.z),radius=0.02, color=camvtk.clColor( p.cc() ) ) )
    
    
    xfibers = bpcx.getFibers()
    yfibers = bpcy.getFibers()
    fibers=xfibers+yfibers
    # sort fibers into different z-levels
    sorted_fibers=[]
    for zh in zvals:
        zlevel_fibers=[]
        for f in fibers:
            if f.p1.z == zh:
                zlevel_fibers.append(f)
        sorted_fibers.append(zlevel_fibers)
    print(" found ",len(sorted_fibers)," z-levels")
    
    n=0
    loops = []
    t2_before = time.time()
    for zlev_fibers in sorted_fibers:
        print(" z-level ",n," at z=", zvals[n], " has ", len(zlev_fibers), "fibers")
        n=n+1
        w = ocl.Weave()
        for f in zlev_fibers:
            w.addFiber(f)
        print(" build()...",)
        w.build()
        print("done")
        print(" split()...",)
        subw = w.get_components()
        print(" graph has ", len(subw)," sub-weaves")
        m=0
        for sw in subw:
            print(m," face_traverse...",)
            t_before = time.time()
            sw.face_traverse()
            t_after = time.time()
            calctime = t_after-t_before
            print("done in ", calctime," s")
            w_loop = sw.getLoops()
            for lop in w_loop:
                loops.append(lop)
            m=m+1
    t2_after = time.time()
    print(" found", len(loops)," loops")
    print(" loop extraction took ", t2_after-t2_before," seconds")
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
                
    t2 = camvtk.Text()
    stltext = "%s\n%i triangles\n%i waterlines\n%i Fibers\n%i CL-points\n%i pushCutter() calls\n%0.1f seconds\n%0.3f us/call"  \
               % ( str(cutter), s.size(), len(zvals), nfibers, len(clpoints), bpc.nCalls, push_calctime, 1e6* push_calctime/bpc.nCalls ) 
    t2.SetText(stltext)
    t2.SetPos( (50, myscreen.height-200) )
    myscreen.addActor( t2)
    
    
    print("done.")
    myscreen.camera.SetPosition(0.5, 3, 2)
    myscreen.camera.SetFocalPoint(0.5, 0.5, 0)
    camvtk.drawArrows(myscreen,center=(-0.5,-0.5,-0.5))
    camvtk.drawOCLtext(myscreen)

    myscreen.render()    

    myscreen.iren.Start()
    #raw_input("Press Enter to terminate") 
