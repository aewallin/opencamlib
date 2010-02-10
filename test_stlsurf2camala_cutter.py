import camala as cam
import pyvtk
import time

if __name__ == "__main__":
    s= cam.STLSurf()
    print s.str()
    myscreen = pyvtk.VTKScreen()
    stl = pyvtk.STLSurf("sphere.stl")
    print "STL surface read"
    myscreen.addActor(stl)
    stl.SetWireframe()
    
    polydata = stl.src.GetOutput()
    pyvtk.vtkPolyData2CamalaSTL(polydata, s)
    
    print s.str()
    c = cam.CylCutter()
    print c
    print c.str()
    print "diam=", c.diameter
    c2 = cam.CylCutter(5.0)
    print c2.str()
    print "diameter=", c2.diameter
    a=cam.Point(0,0,0)
    b=cam.Point(1,2,3)
    c=cam.Point(-1,1,4)
    t = cam.Triangle(a,b,c)
    cc = c2.vertexDrop( cam.Point(0,0,0), t );
    print cc.str()

    myscreen.iren.Start()
    #raw_input("Press Enter to terminate") 
    
