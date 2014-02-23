
# simple parallel finish toolpath example
# Anders Wallin 2014-02-23

import ocl     # https://github.com/aewallin/opencamlib
#import pyocl   # ocl helper library
import camvtk  # ocl helper library

import time
#import vtk
#import datetime       
import math


import ngc_writer # output is produced by this module

# create a simple "Zig" pattern where we cut only in one direction.
# the first line is at ymin
# the last line is at ymax
def YdirectionZigPath(xmin,xmax,ymin,ymax,Ny):
    paths = []
    dy = float(ymax-ymin)/(Ny-1)  # the y step-over
    for n in xrange(0,Ny):
        path = ocl.Path() 
        y = ymin+n*dy              # current y-coordinate 
        if (n==Ny-1):
            assert( y==ymax)
        elif (n==0):
            assert( y==ymin)
        p1 = ocl.Point(xmin,y,0)   # start-point of line
        p2 = ocl.Point(xmax,y,0)   # end-point of line
        l = ocl.Line(p1,p2)        # line-object
        path.append( l )           # add the line to the path
        paths.append(path)
    return paths

def adaptive_path_drop_cutter(surface, cutter, paths):
    apdc = ocl.AdaptivePathDropCutter()
    apdc.setSTL(surface)
    apdc.setCutter(cutter)
    apdc.setSampling(0.04)      # maximum sampling or "step-forward" distance
                                # should be set so that we don't loose any detail of the STL model
                                # i.e. this number should be similar or smaller than the smallest triangle
    apdc.setMinSampling(0.0008) # minimum sampling or step-forward distance
                                # the algorithm subdivides "steep" portions of the toolpath
                                # until we reach this limit.

    cl_paths=[]
    n_points=0
    for path in paths:
        apdc.setPath( path )
        apdc.run()
        cl_points = apdc.getCLPoints()
        n_points = n_points + len( cl_points )
        cl_paths.append( apdc.getCLPoints() )
    return (cl_paths, n_points)

# this could be any source of triangles
# as long as it produces an ocl.STLSurf() we can work with
def STLSurfaceSource(filename):
    stl = camvtk.STLSurf(filename)
    polydata = stl.src.GetOutput()
    s = ocl.STLSurf()
    camvtk.vtkPolyData2OCLSTL(polydata, s)
    return s

# filter a single path
def filter_path(path,tol):
    f = ocl.LineCLFilter()
    f.setTolerance(tol)
    for p in path:
        p2 = ocl.CLPoint(p.x,p.y,p.z)
        f.addCLPoint(p2)
    f.run()
    return  f.getCLPoints()

# to reduce the G-code size we filter here. (this is not strictly required and could be omitted)
# we could potentially detect G2/G3 arcs here, if there was a filter for that.
# idea:
# if a path in the raw toolpath has three points (p1,p2,p3) 
# and point p2 lies within tolerance of the straight line p1-p3
# then we simplify the path to (p1,p3) 
def filterCLPaths(cl_paths, tolerance=0.001):
    cl_filtered_paths = []
    t_before = time.time()
    n_filtered=0
    for cl_path in cl_paths:
        cl_filtered = filter_path(cl_path,tol)
        
        n_filtered = n_filtered + len(cl_filtered)
        cl_filtered_paths.append(cl_filtered)
    return (cl_filtered_paths, n_filtered)

# this uses ngc_writer and writes G-code to stdout or a file
def write_zig_gcode_file(filename, n_triangles, t1,n1,tol,t2,n2, toolpath):
    ngc_writer.clearance_height= 4 # XY rapids at this height
    ngc_writer.feed_height = 0.3   # z plunge-feed below this height
    ngc_writer.feed = 200          # feedrate 
    ngc_writer.plunge_feed = 100   # plunge feedrate
    ngc_writer.metric = False      # metric/inch flag
    ngc_writer.comment( " OpenCAMLib %s" % ocl.version() )
    ngc_writer.comment( " STL surface: %s" % filename )
    ngc_writer.comment( "   triangles: %d" % n_triangles )
    ngc_writer.comment( " OpenCamLib::AdaptivePathDropCutter run took %.2f s" % t1 )
    ngc_writer.comment( " got %d raw CL-points " % n1 )
    ngc_writer.comment( " filtering to tolerance %.4f )" % ( tol )  )
    ngc_writer.comment( " got %d filtered CL-points. Filter done in %.3f s )" % ( n2 , t2 ) )
    ngc_writer.preamble()
    # a "Zig" or one-way parallel finish path
    # 1) lift to clearance height
    # 2) XY rapid to start of path
    # 3) plunge to correct z-depth
    # 4) feed along path until end 
    for path in toolpath:
        ngc_writer.pen_up()  
        first_pt = path[0]
        ngc_writer.xy_rapid_to( first_pt.x, first_pt.y )
        ngc_writer.pen_down( first_pt.z )
        for p in path[1:]:
            ngc_writer.line_to(p.x,p.y,p.z)
    ngc_writer.postamble() # end of program

if __name__ == "__main__":     
    filename = "../../stl/gnu_tux_mod.stl"
    surface = STLSurfaceSource(filename)
    
    diameter=0.25
    length=5
    
    # choose a cutter for the operation:
    # http://www.anderswallin.net/2011/08/opencamlib-cutter-shapes/
    #cutter = ocl.BallCutter(diameter, length)
    cutter = ocl.CylCutter(diameter, length)
    #cutter = ocl.BullCutter(diameter, 0.2, length)
    #angle = math.pi/4
    #cutter = ocl.ConeCutter(diameter, angle, length)
    #cutter = cutter.offsetCutter( 0.4 )
 
    # toolpath is contained in this simple box
    ymin=0
    ymax=12
    xmin=0
    xmax=10
    Ny=40  # number of lines in the y-direction
    paths = YdirectionZigPath(xmin,xmax,ymin,ymax,Ny)

    # now project onto the STL surface
    t_before = time.time()
    (raw_toolpath, n_raw) = adaptive_path_drop_cutter(surface,cutter,paths)
    t1 = time.time()

    tol = 0.001    
    (toolpaths, n_filtered) = filterCLPaths(raw_toolpath, tolerance=0.001)    
    t2 = time.time()-t_before
    
    # output a g-code file
    write_zig_gcode_file( filename, surface.size() , t1, n_raw ,tol,t2,n_filtered, toolpaths )
