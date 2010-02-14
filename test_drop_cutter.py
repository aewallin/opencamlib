import ocl as cam
import camvtk
import time

if __name__ == "__main__":  
    c2 = cam.CylCutter(.1)
    print c2.str()
    a=cam.Point(1,0,0)
    b=cam.Point(0,1,0)
    c=cam.Point(0,0,1)
    t = cam.Triangle(a,b,c)
    print "triangle created  t=", t.str()
    cl = cam.Point(0.2,0.2,0)
    print "CL= ", cl.str()
       
    cc = c2.vertexDrop( cl , t )
    print "after vertex CL=", cl.str()
    print "after vertex CC=", cc.str()
    print "t=",t.str()
    #print "t.n", t.n.str()

    cc2 = c2.facetDrop( cl, t )
    #print "after facetDrop"
    print "after facet t=",t.str()
    #print "t.n", t.n.str()
    print "after facet CL=", cl.str()
    print "after facet CC=", cc2.str()
    print "after facet cl.xyDistance(cc)=", cl.xyDistance(cc2)
    
    cc3 = c2.edgeDrop( cl, t )
    print "after edge cl =", cl.str()
    
    #myscreen.iren.Start()
    #raw_input("Press Enter to terminate") 
    
