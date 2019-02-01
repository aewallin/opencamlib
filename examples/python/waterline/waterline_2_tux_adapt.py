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
        nloop = nloop+1
        

if __name__ == "__main__":  
    print(ocl.version())
    myscreen = camvtk.VTKScreen()
    #stl = camvtk.STLSurf("../../stl/demo.stl")
    stl = camvtk.STLSurf("../../stl/gnu_tux_mod.stl")
    #stl = camvtk.STLSurf("../../stl/waterline1.stl")
    myscreen.addActor(stl)
    stl.SetWireframe() # render tux as wireframe
    #stl.SetSurface() # render tux as surface
    stl.SetColor(camvtk.cyan)
    polydata = stl.src.GetOutput()
    s = ocl.STLSurf()
    camvtk.vtkPolyData2OCLSTL(polydata, s)
    print("STL surface read,", s.size(), "triangles")
    zh=1.75145
    diam = 1.4
    length = 500
    loops = []

    #cutter = ocl.CylCutter( diam , length )
    cutter = ocl.BallCutter( diam , length )
    #cutter = ocl.BullCutter( diam , diam/5, length )
    
    wl = ocl.Waterline()
    wl.setSTL(s)
    wl.setCutter(cutter)
    wl.setZ(zh)
    wl.setSampling(0.5)
    #wl.setThreads(5)
    t_before = time.time() 
    wl.run()
    t_after = time.time()
    calctime = t_after-t_before
    print(" Waterline done in ", calctime," s")
    cutter_loops = wl.getLoops()
    for l in cutter_loops:
        loops.append(l)
    
    aloops = []
    awl = ocl.AdaptiveWaterline()
    awl.setSTL(s)
    awl.setCutter(cutter)
    awl.setZ(zh)
    awl.setSampling(0.1)
    awl.setMinSampling(0.01)
    #wl.setThreads(5)
    t_before = time.time() 
    awl.run()
    t_after = time.time()
    calctime = t_after-t_before
    print(" AdaptiveWaterline done in ", calctime," s")
    acutter_loops = awl.getLoops()
    for l in acutter_loops:
        aloops.append(l)
    
    
    print("All waterlines done. Got", len(loops)," loops in total.")
    # draw the loops
    drawLoops(myscreen, loops, camvtk.yellow)
    drawLoops(myscreen, aloops, camvtk.red)
    
    print("done.")
    myscreen.camera.SetPosition(15, 13, 7)
    myscreen.camera.SetFocalPoint(5, 5, 0)
    camvtk.drawArrows(myscreen,center=(-0.5,-0.5,-0.5))
    camvtk.drawOCLtext(myscreen)
    myscreen.render()    
    myscreen.iren.Start()
    #raw_input("Press Enter to terminate") 
