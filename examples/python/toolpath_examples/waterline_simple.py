
# simple parallel finish toolpath example
# Anders Wallin 2014-02-23

import time
import vtk  # visualization
import math

import ocl        # https://github.com/aewallin/opencamlib
#import pyocl     # ocl helper library
import camvtk     # ocl helper library
import ngc_writer # G-code output is produced by this module





# this could be any source of triangles
# as long as it produces an ocl.STLSurf() we can work with
def STLSurfaceSource(filename):
    stl = camvtk.STLSurf(filename)
    polydata = stl.src.GetOutput()
    s = ocl.STLSurf()
    camvtk.vtkPolyData2OCLSTL(polydata, s)
    return s


# this does not show a toolpath, just the waterlines
# to make a toolpath one would need to:
#  1) decide on an order in which to machine
#     the different z-heights (from lowest to highest, or from highest to lowest)
#  2) within one z-height waterline there can be many loops
#     so decide in what order to machine these
#  3) create plunge/retract moves at the start and end of each loop
#     optionally some fancier lead-in lead-out moves
def vtk_visualize_waterlines(stlfile, waterlines):
    myscreen = camvtk.VTKScreen()
    stl = camvtk.STLSurf(stlfile)
    myscreen.addActor(stl)
    stl.SetSurface() # try also SetWireframe()
    stl.SetColor(camvtk.cyan)
    myscreen.camera.SetPosition(15, 13, 7)
    myscreen.camera.SetFocalPoint(5, 5, 0)
    print("Rendering waterlines at ", len(waterlines), " different z-heights")
    n=0
    for loops in waterlines: # at each z-height, we may get many loops
        print("  %d/%d:" % (n,len(waterlines)))
        drawLoops(myscreen,loops,camvtk.yellow)
        n=n+1
    camvtk.drawArrows(myscreen,center=(-0.5,-0.5,-0.5)) # XYZ coordinate arrows
    camvtk.drawOCLtext(myscreen)
    myscreen.render()    
    myscreen.iren.Start()
    
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
        print("    loop ",nloop, " with ", len(lop), " points")
        nloop = nloop+1

if __name__ == "__main__":     
    stlfile = "../../stl/gnu_tux_mod.stl"
    surface = STLSurfaceSource(stlfile)
    

    t_before = time.time() 
    
    zheights=[0.2, 0.4, 0.6, 0.8, 1.0, 1.2, 1.4, 1.6, 1.8, 2.0, 2.2, 2.4, 2.6] 
    #zheights=[float(1.0)] # for faster computation, calculate only one waterline
    
    wl = ocl.Waterline()          # total time 14 seconds on i7 CPU
    #wl = ocl.AdaptiveWaterline() # this is slower, ca 60 seconds on i7 CPU
    wl.setSTL(surface)
    diam = 0.5
    length= 10 
    cutter = ocl.BallCutter( diam , length ) # any ocl MillingCutter class should work here
    wl.setCutter(cutter)
    wl.setSampling(0.0314) # this should be smaller than the smallest details in the STL file
                           # AdaptiveWaterline() also has settings for minimum sampling interval (see c++ code)
    all_loops=[]
    for zh in zheights:
        print("calculating Waterline at z= ", zh)
        wl.reset()
        wl.setZ(zh) # height for this waterline
        wl.run()
        all_loops.append( wl.getLoops() )
    t_after = time.time()
    calctime = t_after-t_before
    print(" TOTAL Waterline time is: ", calctime," s")
    
    # waterlines around sharp features tend to be circular.
    # however the algorithm outputs only cutter-location points
    # it would be helpful to find a circular-arc filter so we could reduce
    # the amount of data and output G2/G3 arcs instead.
    
    # output a g-code file FIXME: not done yet
    # write_zig_gcode_file( stlfile, surface.size() , t1, n_raw ,tol,t2,n_filtered, toolpaths )
    # and/or visualize with VTK
    vtk_visualize_waterlines(stlfile, all_loops)
