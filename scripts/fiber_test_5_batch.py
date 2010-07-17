import ocl
import camvtk
import time
import vtk
import datetime
import math

if __name__ == "__main__":  
    print ocl.revision()
    
    myscreen = camvtk.VTKScreen()
    
    a = ocl.Point(0,1,0.3)
    myscreen.addActor(camvtk.Point(center=(a.x,a.y,a.z), color=(1,0,1)))
    b = ocl.Point(1,0.5,0.3)    
    myscreen.addActor(camvtk.Point(center=(b.x,b.y,b.z), color=(1,0,1)))
    c = ocl.Point(0,0,0)
    myscreen.addActor(camvtk.Point(center=(c.x,c.y,c.z), color=(1,0,1)))
    myscreen.addActor( camvtk.Line(p1=(a.x,a.y,a.z),p2=(c.x,c.y,c.z)) )
    myscreen.addActor( camvtk.Line(p1=(c.x,c.y,c.z),p2=(b.x,b.y,b.z)) )
    myscreen.addActor( camvtk.Line(p1=(a.x,a.y,a.z),p2=(b.x,b.y,b.z)) )
    t = ocl.Triangle(b,c,a)
    s = ocl.STLSurf()
    s.addTriangle(t) # a one-triangle STLSurf
    
    cutter = ocl.CylCutter(0.3)
    #cutter = ocl.BallCutter(0.3)
        
    cutter.length = 4.0
    print "lengt=", cutter.length
    print "fiber..."
    range=6
    Nmax = 500
    yvals = [float(n-float(Nmax)/2)/Nmax*range for n in xrange(0,Nmax+1)]
    xvals = [float(n-float(Nmax)/2)/Nmax*range for n in xrange(0,Nmax+1)]
    zmin = -0.1
    zmax = 0.5
    zNmax = 50
    dz = (zmax-zmin)/(zNmax-1)
    zvals=[]
    for n in xrange(0,zNmax):
        zvals.append(zmin+n*dz)

    bpc = ocl.BatchPushCutter()
    bpc.setSTL(s)
    bpc.setCutter(cutter)
    # create fibers
    for zh in zvals:
        for y in yvals:
            f1 = ocl.Point(-0.5,y,zh) # start point of fiber
            f2 = ocl.Point(1.5,y,zh)  # end point of fiber
            f =  ocl.Fiber( f1, f2)
            bpc.appendFiber(f)
        for x in xvals:
            f1 = ocl.Point(x,-0.5,zh) # start point of fiber
            f2 = ocl.Point(x,1.5,zh)  # end point of fiber
            f =  ocl.Fiber( f1, f2)
            bpc.appendFiber(f)
            
    # run
    bpc.pushCutter()
    clpoints = bpc.getCLPoints()
    print "rendering raw CL-points."
    
    # draw the CL-points
    camvtk.drawCLPointCloud(myscreen, clpoints)
    print "done."
    myscreen.camera.SetPosition(0.5, 3, 2)
    myscreen.camera.SetFocalPoint(0.5, 0.5, 0)
    camvtk.drawArrows(myscreen,center=(-0.5,-0.5,-0.5))
    camvtk.drawOCLtext(myscreen)

    myscreen.render()    

    myscreen.iren.Start()
    #raw_input("Press Enter to terminate") 
