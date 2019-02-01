import ocl
import camvtk
import time
import vtk
import datetime
import math

def drawLoops(myscreen,loops,loopColor):
    # draw the loops
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
                myscreen.addActor( camvtk.Line(p1=(previous.x,previous.y,previous.z),p2=(p.x,p.y,p.z),color=loopColor) ) # the normal line
                # and a line from p to the first point
                myscreen.addActor( camvtk.Line(p1=(p.x,p.y,p.z),p2=(first_point.x,first_point.y,first_point.z),color=loopColor) )
            else:
                myscreen.addActor( camvtk.Line(p1=(previous.x,previous.y,previous.z),p2=(p.x,p.y,p.z),color=loopColor) )
                previous=p
            n=n+1
        print("rendered loop ",nloop, " with ", len(lop), " points")
        nloop = nloop+1

def getLoops(wl,zh,diam):
    t_before = time.time() 
    wl.reset()
    wl.setZ(zh)
    wl.run()
    t_after = time.time()
    calctime = t_after-t_before
    print(" Waterline done in ", calctime," s")
    return wl.getLoops()


    
if __name__ == "__main__":  
    print(ocl.version())
    myscreen = camvtk.VTKScreen()
    #stl = camvtk.STLSurf("../../stl/demo.stl")
    stl = camvtk.STLSurf("../../stl/gnu_tux_mod.stl")
    myscreen.addActor(stl)
    #stl.SetWireframe() # render tux as wireframe
    stl.SetSurface() # render tux as surface
    stl.SetColor(camvtk.cyan)
    polydata = stl.src.GetOutput()
    s = ocl.STLSurf()
    camvtk.vtkPolyData2OCLSTL(polydata, s)
    print("STL surface read,", s.size(), "triangles")

    #zh = 1.0
    t_before = time.time() 
    diam = 0.5
    zheights=[0.2, 0.4, 0.6, 0.8, 1.0, 1.2, 1.4, 1.6, 1.8, 2.0, 2.2, 2.4, 2.6]
    zheights=[float(1.0)]
    wl = ocl.Waterline()
    #wl = ocl.AdaptiveWaterline()
    wl.setSTL(s)
    length= 10
    cutter = ocl.BallCutter( diam , length )
    wl.setCutter(cutter)
    wl.setSampling(0.0314)
    
    for zh in zheights:
        print("calculating Waterline at z= ", zh)
        cutter_loops = getLoops(wl,zh,diam)
        drawLoops(myscreen,cutter_loops,camvtk.red)
    t_after = time.time()
    calctime = t_after-t_before
    print(" TOTAL Waterline time is: ", calctime," s")
    
    print("done.")
    myscreen.camera.SetPosition(15, 13, 7)
    myscreen.camera.SetFocalPoint(5, 5, 0)
    camvtk.drawArrows(myscreen,center=(-0.5,-0.5,-0.5))
    camvtk.drawOCLtext(myscreen)
    myscreen.render()    
    myscreen.iren.Start()
    #raw_input("Press Enter to terminate") 
