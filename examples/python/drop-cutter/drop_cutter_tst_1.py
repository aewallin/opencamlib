import ocl
import camvtk
import time

if __name__ == "__main__":  
    cutter = ocl.CylCutter(.1, 5)
    print(cutter)
    a = ocl.Point(1,0,0)
    b = ocl.Point(0,1,0)
    c = ocl.Point(0,0,1)
    t = ocl.Triangle(a,b,c)
    print("triangle created  t=", t)
    cl = ocl.CLPoint(0.2,0.2,-5)
    print("CL= ", cl)
    cutter.vertexDrop( cl ,  t )
    print("after vertex CL=", cl)
    print("t=",t)

    #print "t.n", t.n.str()

    cutter.facetDrop( cl, t )
    #print "after facetDrop"
    print("after facet t=",t)
    #print "t.n", t.n.str()
    print("after facet CL=", cl)


    
    cutter.edgeDrop( cl,  t )
    print("after edge cl =", cl)

