import ocl
import camvtk
import time
import vtk
import datetime
import math

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
    #stl.SetWireframe()
    stl.SetSurface()
    stl.SetColor(camvtk.cyan)
    polydata = stl.src.GetOutput()
    s = ocl.STLSurf()
    camvtk.vtkPolyData2OCLSTL(polydata, s)
    print("STL surface read,", s.size(), "triangles")
    bounds = s.getBounds()
    print("STLSurf.Bounds()=", bounds)
    cutter = ocl.CylCutter(0.3,6)
    #cutter = ocl.BallCutter(0.3,6)
        
    cutter.length = 4.0
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
    zNmax = 7
    zvals = generateRange(zmin,zmax,zNmax)
    print(" calculating waterlines at ", len(zvals)," different z-heights")
    #print zvals
    bpc_x = ocl.BatchPushCutter()
    bpc_y = ocl.BatchPushCutter()
    bpc_x.setXDirection()
    bpc_y.setYDirection()
    bpc_x.setSTL(s)
    bpc_y.setSTL(s)
    bpc_x.setCutter(cutter)
    bpc_y.setCutter(cutter)
    # create fibers
    nfibers=0
    for zh in zvals:
        for y in yvals:
            f1 = ocl.Point(xmin,y,zh) # start point of fiber
            f2 = ocl.Point(xmax,y,zh)  # end point of fiber
            f =  ocl.Fiber( f1, f2)
            bpc_x.appendFiber(f)
            
            nfibers=nfibers+1
        for x in xvals:
            f1 = ocl.Point(x,ymin,zh) # start point of fiber
            f2 = ocl.Point(x,ymax,zh)  # end point of fiber
            f =  ocl.Fiber( f1, f2)
            bpc_y.appendFiber(f)
            
            nfibers=nfibers+1
    # run
    #t_before = time.time() 
    #bpc2.pushCutter2()
    #t_after = time.time()
    #push_calctime = t_after-t_before
    #print " BPC done in ", push_calctime," s", bpc2.nCalls," push-calls" 
    #print (push_calctime/bpc2.nCalls)*1e6, " us/call"

    t_before = time.time() 
    bpc_x.run()
    bpc_y.run()
    t_after = time.time()
    push_calctime = t_after-t_before
    print(" BPC 3 done in ", push_calctime," s", bpc_x.getCalls()," push-calls" )
    
        
    clpoints = bpc_x.getCLPoints()
    clp2 = bpc_y.getCLPoints()
    clpoints+=clp2
    print("got ", len(clpoints), " CL-points")
    print("rendering raw CL-points.")
    
    # draw the CL-points
    camvtk.drawCLPointCloud(myscreen, clpoints)
    for p in clpoints:
         myscreen.addActor( camvtk.Sphere(center=(p.x,p.y,p.z),radius=0.02, color=camvtk.clColor( p.cc() ) ) )
    

    
    
    print("done.")
    myscreen.camera.SetPosition(0.5, 3, 2)
    myscreen.camera.SetFocalPoint(0.5, 0.5, 0)
    camvtk.drawArrows(myscreen,center=(-0.5,-0.5,-0.5))
    camvtk.drawOCLtext(myscreen)

    myscreen.render()    

    myscreen.iren.Start()
    #raw_input("Press Enter to terminate") 
