import ocl
import camvtk
import time
import vtk
import datetime
import math


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
        ip1 = f.point( i.lower )
        ip2 = f.point( i.upper )
        myscreen.addActor( camvtk.Line(p1=(ip1.x,ip1.y,ip1.z),p2=(ip2.x,ip2.y,ip2.z), color=camvtk.red) )
        
if __name__ == "__main__":  
    myscreen = camvtk.VTKScreen()
    
    a=ocl.Point(1,0.6,0.5)
    myscreen.addActor(camvtk.Point(center=(a.x,a.y,a.z), color=(1,0,1)))
    b=ocl.Point(0,1,0)    
    myscreen.addActor(camvtk.Point(center=(b.x,b.y,b.z), color=(1,0,1)))
    c=ocl.Point(0,0,0.0)
    myscreen.addActor(camvtk.Point(center=(c.x,c.y,c.z), color=(1,0,1)))
    
    myscreen.addActor( camvtk.Line(p1=(a.x,a.y,a.z),p2=(c.x,c.y,c.z)) )
    myscreen.addActor( camvtk.Line(p1=(c.x,c.y,c.z),p2=(b.x,b.y,b.z)) )
    myscreen.addActor( camvtk.Line(p1=(a.x,a.y,a.z),p2=(b.x,b.y,b.z)) )
    
    f1 = ocl.Point(-2,0.5,-0.2)
    f2 = ocl.Point(2,0.5,-0.2)
    t = ocl.Triangle(b,c,a)
    #radius1=1
    #angle = math.pi/4
    #cutter = ocl.ConeCutter(0.37, angle)
    #cutter = ocl.BallCutter(0.532,5)
    cutter = ocl.CylCutter(0.3,5)
    
    #cutter = ocl.CylConeCutter(0.2,0.5,math.pi/9)
    #cutter = ocl.BallConeCutter(0.4,0.6,math.pi/9)
    #cutter = ocl.BullConeCutter(0.4,0.1,0.7,math.pi/6)
    #cutter = ocl.ConeConeCutter(0.4,math.pi/3,0.7,math.pi/6)
    #cutter = ocl.ConeCutter(0.4, math.pi/3)
    
    print "fiber..."
              
    f = ocl.Fiber( f1, f2)
    i = ocl.Interval()
    f.printInts()  
    print "vertexPush"
    cc = ocl.CCPoint()
    cutter.pushCutter(f,i,t)
    print i
    f.addInterval(i)
    print "AFTER vertexPush"
    f.printInts()  

    #inter = f.getInts()
    #print inter
    print "done."
    
    print "rendering..."
    drawFiber(myscreen, f)
    print "done."
    origo = camvtk.Sphere(center=(0,0,0) , radius=0.1, color=camvtk.blue) 
    origo.SetOpacity(0.2)
    myscreen.addActor( origo )
    
    myscreen.camera.SetPosition(0.5, 3, 2)
    myscreen.camera.SetFocalPoint(0.5, 0.5, 0)
    myscreen.render()
    
    #w2if = vtk.vtkWindowToImageFilter()
    #w2if.SetInput(myscreen.renWin)
    #lwr = vtk.vtkPNGWriter()
    #lwr.SetInput( w2if.GetOutput() )
    
    t = camvtk.Text()
    t.SetPos( (myscreen.width-350, myscreen.height-30) )
    myscreen.addActor(t)
    myscreen.iren.Start()
    #raw_input("Press Enter to terminate") 
