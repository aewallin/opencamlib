import ocl
import camvtk
import time
import vtk
import datetime
import math

def waterline_time(zheights, diam, length,s,sampling):
    t_total = time.time() 
    for zh in zheights:
        cutter = ocl.BallCutter( diam , length )
        wl = ocl.Waterline()
        wl.setSTL(s)
        wl.setCutter(cutter)
        wl.setZ(zh)
        wl.setSampling(sampling)
        wl.setThreads(1)
        
        wl.run()

        cutter_loops = wl.getLoops()
        for l in cutter_loops:
            loops.append(l)
    timeTotal = time.time()-t_total
    print(" ALL Waterlines done in ", timeTotal ," s")
    return timeTotal
    
    
if __name__ == "__main__":  
    print(ocl.version())
    a = ocl.Point(0,1,0.3)
    b = ocl.Point(1,0.5,0.3)    
    c = ocl.Point(0,0,0)
    t = ocl.Triangle(b,c,a)
    s = ocl.STLSurf()
    s.addTriangle(t) # a one-triangle STLSurf
    
    # alternatively, run on the tux model
    stl = camvtk.STLSurf("../../stl/gnu_tux_mod.stl")
    #myscreen.addActor(stl)
    #stl.SetWireframe() # render tux as wireframe
    #stl.SetSurface() # render tux as surface
    #stl.SetColor(camvtk.cyan)
    polydata = stl.src.GetOutput()
    s = ocl.STLSurf()
    camvtk.vtkPolyData2OCLSTL(polydata, s)
    
    zheights=[-0.3, -0.2, -0.1, -0.05, 0.0, 0.05, 0.1, 0.15, 0.2, 0.25, 0.26, 0.27, 0.28, 0.29 ]  # the z-coordinates for the waterlines
    zheights=[-0.8, -0.7, -0.6, -0.5, -0.4, -0.3, -0.2, -0.1, -0.05, 0.0, 0.05, 0.1, 0.15, 0.2,  0.28 ]
    zheights=[ -0.05, 0.0, 0.05, 0.1, 0.15, 0.2, 0.28]
    zheights=[ 1.75145 ]
    diam = 0.6 # run the thing for all these cutter diameters
    length = 5
    loops = []
    cutter = ocl.CylCutter( 1 , 1 )   
    sampling=0.005
    waterline_time(zheights, diam, length,s,sampling)

