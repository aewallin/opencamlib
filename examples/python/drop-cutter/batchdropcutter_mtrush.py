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
    
    #stl = camvtk.STLSurf("../stl/gnu_tux_mod.stl")
    stl = camvtk.STLSurf("../stl/mount_rush.stl") 
    #stl = camvtk.STLSurf("../stl/pycam-textbox.stl") 
    myscreen.addActor(stl)
    stl.SetWireframe()
    stl.SetColor((0.5,0.5,0.5))
    
    polydata = stl.src.GetOutput()
    s = ocl.STLSurf()
    camvtk.vtkPolyData2OCLSTL(polydata, s)
    print("STL surface with", s.size(), "triangles read")
    
    # define a cutter
    length=5
    cutter = ocl.BallCutter(15.4321, length)
    #cutter = ocl.CylCutter(1.123, length)
    #cutter = ocl.BullCutter(1.123, 0.2, length)
    #cutter = ocl.ConeCutter(0.43, math.pi/7, length)
    
    
    print(cutter)
    
    #define grid of CL-points
    minx=-42
    dx=0.5
    maxx=47
    miny=-27
    dy=0.2
    maxy=20
    z=-55
    clpoints = pyocl.CLPointGrid(minx,dx,maxx,miny,dy,maxy,z)
    print("generated grid with", len(clpoints)," CL-points")
    
    # batchdropcutter    
    bdc1 = ocl.BatchDropCutter()
    bdc1.setSTL(s)
    bdc1.setCutter(cutter)
    
    # push the points to ocl
    for p in clpoints:
        bdc1.appendPoint(p)
   
    # run the actual calculation
    t_before = time.time()    
    bdc1.run()
    t_after = time.time()
    calctime = t_after-t_before
    print(" done in ", calctime," s"    )

    
    # get back results from ocl
    clpts = bdc1.getCLPoints()

    
    # draw the results
    print("rendering...",)
    camvtk.drawCLPointCloud(myscreen, clpts)
    print("done")
    
    myscreen.camera.SetPosition(25, 23, 15)
    myscreen.camera.SetFocalPoint(4, 5, 0)
    
    # ocl text
    t = camvtk.Text()
    t.SetText("OpenCAMLib")
    t.SetPos( (myscreen.width-200, myscreen.height-30) )
    myscreen.addActor( t)
    
    # other text
    t2 = camvtk.Text()
    stltext = "%i triangles\n%i CL-points\n%0.1f seconds" % (s.size(), len(clpts), calctime)
    t2.SetText(stltext)
    t2.SetPos( (50, myscreen.height-100) )
    myscreen.addActor( t2)
    
    t3 = camvtk.Text()
    ctext = "Cutter: %s" % ( str(cutter) )
    
    t3.SetText(ctext)
    t3.SetPos( (50, myscreen.height-150) )
    myscreen.addActor( t3)
    
    myscreen.render()
    myscreen.iren.Start()
    raw_input("Press Enter to terminate") 
