import ocl
import camvtk
import time


if __name__ == "__main__":
    p = ocl.Point()
    p.x=7
    p.y=8
    p.z=-3
    print(p)
    q = ocl.Point(1,2,3)
    r = p + q
    t = ocl.Triangle(p,q,r)
    print(t)
    s= ocl.STLSurf()
    print(s)
    s.addTriangle(t)
    s.addTriangle(t)
    print(s)
    print("end.")
    
    myscreen = camvtk.VTKScreen()
    print("screen created")
    stl = camvtk.STLSurf("../stl/sphere.stl")
    print("STL surface read")
    myscreen.addActor(stl)
    
    b = stl.src.GetOutput()
    print(b)
    print("Verts:",b.GetNumberOfVerts())
    print("Cells:",b.GetNumberOfCells())
    print("Lines:",b.GetNumberOfLines())
    print("Polys:",b.GetNumberOfPolys())
    print("Strips:",b.GetNumberOfStrips())
    c = b.GetCell(0)
    print(c)
    print("Points:",c.GetNumberOfPoints())
    print("Edges:",c.GetNumberOfEdges())
    print("Faces:",c.GetNumberOfFaces())
    ps = c.GetPoints()
    print(ps)
    n=ps.GetNumberOfPoints()
    print("Nr of Points:",n)
    for id in range(0,n):
        print(id,"=",)
        print(ps.GetPoint(id))
        myscreen.addActor( camvtk.Sphere(radius=0.5,center=ps.GetPoint(id)) )
    myscreen.render()
    myscreen.iren.Start()
    #raw_input("Press Enter to terminate") 
    
