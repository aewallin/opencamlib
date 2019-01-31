import ocl
import camvtk
import time
import vtk
import datetime
import math

def drawLoops(myscreen, loops, loopcolor):
    nloop = 0
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
                myscreen.addActor( camvtk.Line(p1=(previous.x,previous.y,previous.z),p2=(p.x,p.y,p.z),color=loopcolor) ) # the normal line
                # and a line from p to the first point
                myscreen.addActor( camvtk.Line(p1=(p.x,p.y,p.z),p2=(first_point.x,first_point.y,first_point.z),color=loopcolor) )
            else:
                myscreen.addActor( camvtk.Line(p1=(previous.x,previous.y,previous.z),p2=(p.x,p.y,p.z),color=loopcolor) )
                previous=p
            n=n+1
        print("rendered loop ",nloop, " with ", len(lop), " points")
        if len(lop) == 2:
            for p in lop:
                print(p)
                myscreen.addActor( camvtk.Sphere(center=(p.x,p.y,p.z),radius=0.1,color=camvtk.green) )
        nloop = nloop+1
        

if __name__ == "__main__":  
    print(ocl.version())
    myscreen = camvtk.VTKScreen()
    #stl = camvtk.STLSurf("../../stl/demo.stl")
    #stl = camvtk.STLSurf("../../stl/gnu_tux_mod.stl")
    #stl = camvtk.STLSurf("../../stl/porche.stl")
    #stl = camvtk.STLSurf("../../stl/ktoolcav.stl")
    #stl = camvtk.STLSurf("../../stl/ktoolcor.stl")
    stl = camvtk.STLSurf("../../stl/sphere_cutout.stl")
    #myscreen.addActor(stl)
    #stl.SetWireframe() # render tux as wireframe
    #stl.SetSurface() # render tux as surface
    #stl.SetColor(camvtk.cyan)
    polydata = stl.src.GetOutput() # get polydata from vtk-surface
    s = ocl.STLSurf()
    camvtk.vtkPolyData2OCLSTL(polydata, s) #put triangles on ocl-surface
    #s.rotate(-math.pi/2,math.pi,0)
    stl2 = camvtk.STLSurf(triangleList= s.getTriangles() )
    myscreen.addActor(stl2) 
    stl2.SetSurface()
    stl2.SetColor(camvtk.cyan)
    print("STL surface read,", s.size(), "triangles")
    zh=-0.5
    zheights=[ -0.7, -0.6, -0.5, -0.4, -0.3, -0.2, -0.1, -0.05] # for cavity
    zheights=[ -0.1, 0.0, 0.1, 0.2, 0.3, 0.4 , 0.5, 0.6, 0.7] # for core
    zheights=[ 10, 20, 30, 40, 50, 60] # for waterline1.stl
    diam = 6
    length = 100
    loops = []

    #cutter = ocl.CylCutter( diam , length )
    cutter = ocl.BallCutter( diam , length )
    #cutter = ocl.BullCutter( diam , diam/5, length )
    """
    wl = ocl.Waterline()
    wl.setSTL(s)
    wl.setCutter(cutter)
    wl.setZ(zh)
    wl.setSampling(0.02)
    #wl.setThreads(5)
    t_before = time.time() 
    wl.run2()
    t_after = time.time()
    calctime = t_after-t_before
    print(" Waterline done in ", calctime," s")
    cutter_loops = wl.getLoops()
    for l in cutter_loops:
        loops.append(l)
    """
    sampling=1
    minSampling=0.1
    
    aloops = []
    for zh in zheights:
        awl = ocl.AdaptiveWaterline()
        awl.setSTL(s)
        awl.setCutter(cutter)
        awl.setZ(zh)
        awl.setSampling(sampling)
        awl.setMinSampling(minSampling)
        #wl.setThreads(5)
        t_before = time.time() 
        awl.run()
        t_after = time.time()
        calctime = t_after-t_before
        print(" AdaptiveWaterline done in ", calctime," s")
        acutter_loops = awl.getLoops()
        for l in acutter_loops:
            aloops.append(l)
    
    drawLoops(myscreen, aloops, camvtk.red)
    
    print("done.")
    myscreen.camera.SetPosition(185, 153, 167)
    myscreen.camera.SetFocalPoint(5, 5, 0)
    camvtk.drawArrows(myscreen,center=(0,-4,0))
    camvtk.drawOCLtext(myscreen)
    myscreen.render()    
    myscreen.iren.Start()
    #raw_input("Press Enter to terminate") 
