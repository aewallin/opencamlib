import ocl
import pyocl
import camvtk
import time
import vtk
import datetime
import math

if __name__ == "__main__":  
    print(ocl.version())
    myscreen = camvtk.VTKScreen()
    
    # read STL file from disk
    stl = camvtk.STLSurf("../../../stl/gnu_tux_mod.stl")
    #stl = camvtk.STLSurf("../stl/beet_mm.stl")
    #stl = camvtk.STLSurf("../stl/Blade.stl")
    myscreen.addActor(stl)
    stl.SetWireframe()
    stl.SetColor((0.5,0.5,0.5))
    
    polydata = stl.src.GetOutput()
    s = ocl.STLSurf()
    camvtk.vtkPolyData2OCLSTL(polydata, s)
    print("STL surface read ", s.size(), " triangles")
    
    # Define a cutter
    length=5
    cutter = ocl.BallCutter(1.4321, length)
    #cutter = ocl.CylCutter(1.123, length)
    #cutter = ocl.BullCutter(1.4123, 0.5, length)
    #cutter = ocl.ConeCutter(0.43, math.pi/7, length)
    print(cutter)
    
    # a grid of XY points where we run drop-cutter
    minx, dx, maxx = 0, 0.006, 9
    miny, dy, maxy = 0, 0.1, 12
    z=-5
    clpoints = pyocl.CLPointGrid(minx,dx,maxx,miny,dy,maxy,z)
    print("generated grid with", len(clpoints)," CL-points")
    
    # batchdropcutter    
    bdc1 = ocl.BatchDropCutter()
    bdc1.setSTL(s)
    bdc1.setCutter(cutter)
    
    for p in clpoints:
        bdc1.appendPoint(p)
        
    t_before = time.time()    
    bdc1.run() # the actual drop-cutter run
    t_after = time.time()
    calctime = t_after-t_before
    print(" done in %f s" % calctime    )
    print(" time/point %g s" % (calctime/len(clpoints))   )
    
    clpts = bdc1.getCLPoints()
    print("rendering...")
    camvtk.drawCLPointCloud(myscreen, clpts)
    print("done")
       
    myscreen.camera.SetPosition(3, 23, 15)
    myscreen.camera.SetFocalPoint(4, 5, 0)
    
    t = camvtk.Text()
    t.SetText("OpenCAMLib")
    t.SetPos( (myscreen.width-200, myscreen.height-30) )
    myscreen.addActor( t)
    
    t2 = camvtk.Text()
    stltext = "%i triangles\n%i CL-points\n%0.1f seconds" % (s.size(), len(clpts), calctime)
    t2.SetText(stltext)
    t2.SetPos( (50, myscreen.height-200) )
    myscreen.addActor( t2)
    
    t3 = camvtk.Text()
    ctext = "Cutter: %s" % ( str(cutter) )
    
    t3.SetText(ctext)
    t3.SetPos( (50, myscreen.height-250) )
    myscreen.addActor( t3)
    
    myscreen.render()
    myscreen.iren.Start()
    raw_input("Press Enter to terminate") 
