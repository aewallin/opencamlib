import ocl
import camvtk
import time

if __name__ == "__main__":  
    cutter = ocl.CylCutter(.1)
    print cutter
    a = ocl.Point(1,0,0)
    b = ocl.Point(0,1,0)
    c = ocl.Point(0,0,1)
    t = ocl.Triangle(a,b,c)
    print "triangle created  t=", t
    cl = ocl.Point(0.2,0.2,0)
    cc = ocl.CCPoint()
    print "CL= ", cl
    print "CC=", cc   
    cutter.vertexDrop( cl , cc, t )
    print "after vertex CL=", cl
    print "after vertex CC=", cc
    print "t=",t
    #print "t.n", t.n.str()

    cutter.facetDrop( cl, cc, t )
    #print "after facetDrop"
    print "after facet t=",t
    #print "t.n", t.n.str()
    print "after facet CL=", cl
    print "after facet CC=", cc
    print "after facet cl.xyDistance(cc)=", cl.xyDistance(ocl.Point(cc.x,cc.y,cc.z))
    
    cutter.edgeDrop( cl, cc, t )
    print "after edge cl =", cl
    cc2=ocl.CCPoint()
    cutter.dropCutter(cl,cc2,t)
    #myscreen.iren.Start()
    #raw_input("Press Enter to terminate") 
