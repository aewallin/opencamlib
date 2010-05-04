import ocl as cam
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
            plist.append( cam.Point(x,y,z) )
    return plist

def drawPoints(myscreen, clpoints, ccpoints):
    c=camvtk.PointCloud( pointlist=clpoints, collist=ccpoints) 
    c.SetPoints()
    myscreen.addActor(c )

def drawFiber(myscreen, f):
    #myscreen.addActor( camvtk.Line(p1=(f.p1.x,f.p1.y,f.p1.z),p2=(f.p2.x,f.p2.y,f.p2.z), color=camvtk.orange) )
    #myscreen.addActor( camvtk.Sphere(center=(f.p1.x,f.p1.y,f.p1.z),radius=0.05, color=camvtk.lgreen) )
    #myscreen.addActor( camvtk.Sphere(center=(f.p2.x,f.p2.y,f.p2.z),radius=0.05, color=camvtk.pink) )
    inter = f.getInts()
    for i in inter:
        ip1 = f.point( i[0] )
        ip2 = f.point( i[1] )
        myscreen.addActor( camvtk.Line(p1=(ip1.x,ip1.y,ip1.z),p2=(ip2.x,ip2.y,ip2.z), color=camvtk.red) )
        
if __name__ == "__main__":  
    myscreen = camvtk.VTKScreen()
    
    a=cam.Point(1,0.6,0.5)
    myscreen.addActor(camvtk.Point(center=(a.x,a.y,a.z), color=(1,0,1)))
    b=cam.Point(0,1,0)    
    myscreen.addActor(camvtk.Point(center=(b.x,b.y,b.z), color=(1,0,1)))
    c=cam.Point(0,0,0.0)
    myscreen.addActor(camvtk.Point(center=(c.x,c.y,c.z), color=(1,0,1)))
    
    myscreen.addActor( camvtk.Line(p1=(a.x,a.y,a.z),p2=(c.x,c.y,c.z)) )
    myscreen.addActor( camvtk.Line(p1=(c.x,c.y,c.z),p2=(b.x,b.y,b.z)) )
    myscreen.addActor( camvtk.Line(p1=(a.x,a.y,a.z),p2=(b.x,b.y,b.z)) )
    
    f1 = cam.Point(-2,0.5,-0.2)
    f2 = cam.Point(2,0.5,-0.2)
    
    
    
    t = cam.Triangle(b,c,a)
    #radius1=1
    #angle = math.pi/4
    #cutter = cam.ConeCutter(0.37, angle)
    #cutter = cam.BallCutter(0.532)
    cutter = cam.CylCutter(0.3)
    cutter.length = 4.0
    #cutter = cam.CylConeCutter(0.2,0.5,math.pi/9)
    #cutter = cam.BallConeCutter(0.4,0.6,math.pi/9)
    #cutter = cam.BullConeCutter(0.4,0.1,0.7,math.pi/6)
    #cutter = cam.ConeConeCutter(0.4,math.pi/3,0.7,math.pi/6)
    #cutter = cam.ConeCutter(0.4, math.pi/3)
    
    print "fiber..."
              
    f = cam.Fiber( f1, f2)
    print f.dir.str() 
    
    #f.addInt( 0.25, 0.37)
    #f.addInt( 0.38, 0.47)
    #f.addInt( 0.4, 0.41)
    #f.addInt( 0.2 , 0.3)
    #f.addInt( 0.1, 0.2)
    f.printInts()  
    print "vertexPush"
    cutter.vertexPush(f,t)
    print "AFTER vertexPush"
    f.printInts()  
    print "condense()"
    f.condense()
    
    f.printInts()
    print "condense() again"
    f.condense()
    print "result:"
    f.printInts()
    
    inter = f.getInts()
    print inter
    
    drawFiber(myscreen, f)
            
    print "done."
    
    print "rendering..."

    
    #drawPoints(myscreen, clpoints, ccpoints)

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
    
