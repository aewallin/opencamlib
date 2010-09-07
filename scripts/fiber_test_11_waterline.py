import ocl
import camvtk
import time
import vtk
import datetime
import math

def generateRange(zmin,zmax,zNmax):
    if zNmax>1:
        dz = (float(zmax)-float(zmin))/(zNmax-1)
    else:
        dz = 0
    zvals=[]
    for n in xrange(0,zNmax):
        zvals.append(zmin+n*dz)
    return zvals
        
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
    zh=1.75145
    cutter_diams = generateRange(0.4, 3, 3)
    length = 5
    loops = []
    for diam in cutter_diams:
        #cutter = ocl.CylCutter( diam , length )
        cutter = ocl.BallCutter( diam , length )
        wl = ocl.Waterline()
        wl.setSTL(s)
        wl.setCutter(cutter)
        wl.setZ(zh)
        wl.setTolerance(0.04)
        wl.run()
        cutter_loops = wl.getLoops()
        for l in cutter_loops:
            loops.append(l)

    print "All waterlines done. Got", len(loops)," loops in total."
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
