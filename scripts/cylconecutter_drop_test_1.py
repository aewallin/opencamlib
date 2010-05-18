import ocl
import camvtk
import time
import vtk
import datetime
import math

def CLPointGrid(minx,dx,maxx,miny,dy,maxy,z):
    plist = []
    xvalues = [round(minx+n*dx,2) for n in xrange(int(round((maxx-minx)/dx))+1) ]
    yvalues = [round(miny+n*dy,2) for n in xrange(int(round((maxy-miny)/dy))+1) ]
    for y in yvalues:
        for x in xvalues:
            plist.append( ocl.Point(x,y,z) )
    return plist

def drawPoints(myscreen, clpoints, ccpoints):
    c=camvtk.PointCloud( pointlist=clpoints, collist=ccpoints) 
    c.SetPoints()
    myscreen.addActor(c )
        

if __name__ == "__main__":  
    myscreen = camvtk.VTKScreen()
    
    a=ocl.Point(1,0.6,0.1)
    myscreen.addActor(camvtk.Point(center=(a.x,a.y,a.z), color=(1,0,1)))
    b=ocl.Point(0,1,0)    
    myscreen.addActor(camvtk.Point(center=(b.x,b.y,b.z), color=(1,0,1)))
    c=ocl.Point(0,0,0.0)
    myscreen.addActor(camvtk.Point(center=(c.x,c.y,c.z), color=(1,0,1)))
    
    myscreen.addActor( camvtk.Line(p1=(a.x,a.y,a.z),p2=(c.x,c.y,c.z)) )
    myscreen.addActor( camvtk.Line(p1=(c.x,c.y,c.z),p2=(b.x,b.y,b.z)) )
    myscreen.addActor( camvtk.Line(p1=(a.x,a.y,a.z),p2=(b.x,b.y,b.z)) )
    
    t = ocl.Triangle(b,c,a)
    radius1=1
    angle = math.pi/4
    #cutter = ocl.ConeCutter(0.37, angle)
    #cutter = ocl.BallCutter(0.532)
    #cutter = ocl.CylCutter(0.3)
    #cutter = ocl.BullCutter(1,0.2)
    
    #cutter = ocl.CylConeCutter(0.2,0.5,math.pi/9)
    cutter =  ocl.BallConeCutter(0.4,0.6,math.pi/9)
    #cutter = ocl.BullConeCutter(0.4,0.1,0.7,math.pi/6)
    #cutter = ocl.ConeConeCutter(0.4,math.pi/3,0.7,math.pi/6)
    #cutter = ocl.ConeCutter(0.4, math.pi/3)
    print cutter
    
    
    #print cc.type
    minx=-0.5
    dx=0.0051
    maxx=1.5
    miny=-0.7
    dy=dx
    maxy=1.5
    z=-1.8
    clpoints = CLPointGrid(minx,dx,maxx,miny,dy,maxy,z)
    nv=0
    nn=0
    ne=0
    nf=0
    print len(clpoints), "cl-points to evaluate"
    n=0
    ccpoints=[]
    
    for cl in clpoints:
        cc = ocl.CCPoint()
        cutter.vertexDrop(cl,cc,t)
        #cutter.edgeDrop(cl,cc,t)
        #cutter.facetDrop(cl,cc,t)
        #cutter.dropCutter(cl,cc,t)
        ccpoints.append(cc)
        n=n+1
        if (n % int(len(clpoints)/10)) == 0:
            print n/int(len(clpoints)/10), " ",
              
    print "done."
    print "rendering..."
    print " len(clpoints)=", len(clpoints)
    print " len(ccpoints)=", len(ccpoints)
    drawPoints(myscreen, clpoints, ccpoints)
    print "done."
    origo = camvtk.Sphere(center=(0,0,0) , radius=0.1, color=camvtk.blue) 
    origo.SetOpacity(0.2)
    myscreen.addActor( origo )
     
    
    myscreen.camera.SetPosition(0.5, 3, 2)
    myscreen.camera.SetFocalPoint(0.5, 0.5, 0)
    myscreen.render()
    
    w2if = vtk.vtkWindowToImageFilter()
    w2if.SetInput(myscreen.renWin)
    lwr = vtk.vtkPNGWriter()
    lwr.SetInput( w2if.GetOutput() )
    
    t = camvtk.Text()
    t.SetPos( (myscreen.width-350, myscreen.height-30) )
    myscreen.addActor(t)
    myscreen.iren.Start()
    #raw_input("Press Enter to terminate") 
