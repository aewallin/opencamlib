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
        myscreen.addActor( camvtk.Sphere(center=(ip1.x,ip1.y,ip1.z),radius=0.005, color=camvtk.ccColor( i.lower_cc) ) )
        myscreen.addActor( camvtk.Sphere(center=(ip2.x,ip2.y,ip2.z),radius=0.005, color=camvtk.ccColor( i.upper_cc) ) )
        
if __name__ == "__main__":  
    myscreen = camvtk.VTKScreen()
    
    a = ocl.Point(1,0.6,0.3)
    myscreen.addActor(camvtk.Point(center=(a.x,a.y,a.z), color=(1,0,1)))
    b = ocl.Point(0,1,0)    
    myscreen.addActor(camvtk.Point(center=(b.x,b.y,b.z), color=(1,0,1)))
    c = ocl.Point(0,0,0.0)
    myscreen.addActor(camvtk.Point(center=(c.x,c.y,c.z), color=(1,0,1)))
    myscreen.addActor( camvtk.Line(p1=(a.x,a.y,a.z),p2=(c.x,c.y,c.z)) )
    myscreen.addActor( camvtk.Line(p1=(c.x,c.y,c.z),p2=(b.x,b.y,b.z)) )
    myscreen.addActor( camvtk.Line(p1=(a.x,a.y,a.z),p2=(b.x,b.y,b.z)) )
    t = ocl.Triangle(b,c,a)

    cutter = ocl.CylCutter(0.3)
    cutter.length = 4.0
    print "lengt=", cutter.length
    print "fiber..."
    range=4
    Nmax = 200
    yvals = [float(n-float(Nmax)/2)/Nmax*range for n in xrange(0,Nmax+1)]
    
    #cc = ocl.CCPoint()
    zh = 0.1
    for y in yvals:
        f1 = ocl.Point(-2,y,zh) # start point of fiber
        f2 = ocl.Point(2,y,zh)  # end point of fiber
        f =  ocl.Fiber( f1, f2)
        i = ocl.Interval()
        cutter.vertexPush(f,i,t)
        cutter.facetPush(f,i,t)
        cutter.edgePush(f,i,t)
        f.addInterval(i)
        #f.printInts()  
    
        drawFiber(myscreen, f)
            
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
