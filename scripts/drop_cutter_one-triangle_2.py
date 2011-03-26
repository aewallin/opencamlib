import ocl
import pyocl
import camvtk
import time
import vtk
import datetime
import math


def drawPoints(myscreen, clpoints, ccpoints):
    c=camvtk.PointCloud( pointlist=clpoints, collist=ccpoints) 
    c.SetPoints()
    myscreen.addActor(c )
        

if __name__ == "__main__":  
    print ocl.revision()
    myscreen = camvtk.VTKScreen()
    
    a=ocl.Point(1,0,0.4)
    myscreen.addActor(camvtk.Point(center=(a.x,a.y,a.z), color=(1,0,1)))
    b=ocl.Point(0,1,0)    
    myscreen.addActor(camvtk.Point(center=(b.x,b.y,b.z), color=(1,0,1)))
    c=ocl.Point(0,0,-0.2)
    myscreen.addActor(camvtk.Point(center=(c.x,c.y,c.z), color=(1,0,1)))
    
    myscreen.addActor( camvtk.Line(p1=(a.x,a.y,a.z),p2=(c.x,c.y,c.z)) )
    myscreen.addActor( camvtk.Line(p1=(c.x,c.y,c.z),p2=(b.x,b.y,b.z)) )
    myscreen.addActor( camvtk.Line(p1=(a.x,a.y,a.z),p2=(b.x,b.y,b.z)) )
    
    t = ocl.Triangle(b,c,a)
    radius1=1
    length=5
    angle = math.pi/4
    #cutter = ocl.ConeCutter(0.37, angle)
    cutter = ocl.BallCutter(0.532, length)
    #cutter = ocl.CylCutter(0.3, length)
    #cutter = ocl.BullCutter(0.5,0.123, length)
    print cutter
    
    
    #print cc.type
    minx=-0.5
    dx=0.0051
    maxx=1.5
    miny=-0.7
    dy=dx
    maxy=1.5
    z=-0.7
    clpoints = pyocl.CLPointGrid(minx,dx,maxx,miny,dy,maxy,z)

    print len(clpoints), "cl-points to evaluate"
    n=0
    for cl in clpoints:
        #cutter.vertexDrop(cl,t)
        #cutter.edgeDrop(cl,t)
        #cutter.facetDrop(cl,t)
        cutter.dropCutter(cl,t) # this calls all three above: vertex,facet,edge
        n=n+1
        if (n % int(len(clpoints)/10)) == 0:
            print n/int(len(clpoints)/10), " ",
              
    print "done."
    
    print "rendering..."
    print " len(clpoints)=", len(clpoints)

    camvtk.drawCLPointCloud(myscreen, clpoints)
    print "done."
    
    origo = camvtk.Sphere(center=(0,0,0) , radius=0.1, color=camvtk.blue) 
    origo.SetOpacity(0.2)
    myscreen.addActor( origo )
     
    myscreen.camera.SetPosition(0.5, 3, 2)
    myscreen.camera.SetFocalPoint(0.5, 0.5, 0)
    myscreen.camera.SetClippingRange(-20,20)
    myscreen.render()
    myscreen.iren.Start()
    #raw_input("Press Enter to terminate") 
