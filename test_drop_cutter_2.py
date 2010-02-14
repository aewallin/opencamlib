import ocl as cam
import camvtk
import time

if __name__ == "__main__":  
    cutter = cam.CylCutter(.1)
    print cutter.str()
    a=cam.Point(1,0,0)
    b=cam.Point(0,1,0)
    c=cam.Point(0,0,1)
    t = cam.Triangle(a,b,c)
    print "triangle created  t=", t.str()
    cl = cam.Point(0.2,0.2,0)
    cc = cam.CCPoint()
    cutter.dropCutter(cl,cc,t)
    print "CL= ", cl.str()
    print "CC=", cc.str()
       
    #c2.vertexDrop( cl , cc, t )
    #print "after vertex CL=", cl.str()
    #print "after vertex CC=", cc.str()
    #print "t=",t.str()
    #print "t.n", t.n.str()
    #c2.facetDrop( cl, cc, t )
    #print "after facetDrop"
    #print "after facet t=",t.str()
    #print "t.n", t.n.str()
    #print "after facet CL=", cl.str()
    #print "after facet CC=", cc.str()
    #print "after facet cl.xyDistance(cc)=", cl.xyDistance(cc)
    #c2.edgeDrop( cl, cc, t )
    #print "after edge cl =", cl.str()
    #cc2=cam.Point()
    #myscreen.iren.Start()
    #raw_input("Press Enter to terminate") 
    
