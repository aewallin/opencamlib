import ocl as cam
import camvtk
import time

if __name__ == "__main__":
    #s= cam.STLSurf()
    #print s.str()
    #myscreen = camvtk.VTKScreen()
    #stl = camvtk.STLSurf("sphere.stl")
    #print "STL surface read"
    #myscreen.addActor(stl)
    #stl.SetWireframe()
    
    #polydata = stl.src.GetOutput()
    #camvtk.vtkPolyData2OCLSTL(polydata, s)
    
    #print s.str()
    
    c = cam.CylCutter()
    print c.str()
    print "diam=", c.diameter
    c2 = cam.CylCutter(.1)
    print c2.str()
    print "diameter=", c2.diameter
    a=cam.Point(1,0,0)
    b=cam.Point(0,1,0)
    c=cam.Point(0,0,1)
    t = cam.Triangle(a,b,c)
    print "triangle created  t=", t.str()
    cl = cam.Point(0.01,0.3,0)
    print "CL= ", cl.str()
       
    cc = c2.vertexDrop( cl , t )
    print "vertex CL=", cl.str()
    print "vertex CC=", cc.str()
    print "before FacetDrop"
    print "t=",t.str()
    #print "t.n", t.n.str()

    cc2 = c2.facetDrop( cl, t)
    print "after facetDrop"
    print "t=",t.str()
    #print "t.n", t.n.str()
    print "facet CL=", cl.str()
    print "facet CC=", cc2.str()
    print "cl.xyDistance(cc)=", cl.xyDistance(cc2)
    
    cc3 = c2.edgeDrop(cl, t)
    print "after edgeDrop"
    print "cl =", cl.str()
    
    #myscreen.iren.Start()
    #raw_input("Press Enter to terminate") 
    
