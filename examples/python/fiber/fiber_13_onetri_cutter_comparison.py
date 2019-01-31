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

def drawFiber(myscreen, f, fibercolor):
    inter = f.getInts()
    print("fiber has ", len(inter) , " intervals")
    for i in inter:
        if not i.empty():
            ip1 = f.point( i.lower )
            ip2 = f.point( i.upper )
            myscreen.addActor( camvtk.Line(p1=(ip1.x,ip1.y,ip1.z),p2=(ip2.x,ip2.y,ip2.z), color=fibercolor) )
            myscreen.addActor( camvtk.Sphere(center=(ip1.x,ip1.y,ip1.z),radius=0.005, color=camvtk.clColor( i.lower_cc) ) )
            myscreen.addActor( camvtk.Sphere(center=(ip2.x,ip2.y,ip2.z),radius=0.005, color=camvtk.clColor( i.upper_cc) ) )
            #cc1 = i.lower_cc
            #cc2 = i.upper_cc
            #myscreen.addActor( camvtk.Sphere(center=(cc1.x,cc1.y,cc1.z),radius=0.005, color=camvtk.lgreen ) )
            #myscreen.addActor( camvtk.Sphere(center=(cc2.x,cc2.y,cc2.z),radius=0.005, color=camvtk.lgreen ) )
            # cutter circle
            #c1 = camvtk.Circle(center=(ip1.x,ip1.y,ip1.z), radius = 0.3/2, color=fibercolor)
            #myscreen.addActor(c1)
            #c2 = camvtk.Circle(center=(ip2.x,ip2.y,ip2.z), radius = 0.3/2, color=fibercolor)
            #myscreen.addActor(c2)

def drawFiber_clpts(myscreen, f, clcolor):
    inter = f.getInts()
    #print "fiber has ", len(inter) , " intervals"
    for i in inter:
        if not i.empty():
            ip1 = f.point( i.lower )
            ip2 = f.point( i.upper )
            #myscreen.addActor( camvtk.Line(p1=(ip1.x,ip1.y,ip1.z),p2=(ip2.x,ip2.y,ip2.z), color=fibercolor) )
            sphcolor = camvtk.clColor( i.lower_cc) 
            myscreen.addActor( camvtk.Sphere(center=(ip1.x,ip1.y,ip1.z),radius=0.005, color=sphcolor ) )
            sphcolor = camvtk.clColor( i.upper_cc)
            myscreen.addActor( camvtk.Sphere(center=(ip2.x,ip2.y,ip2.z),radius=0.005, color=sphcolor ) )
            #cc1 = i.lower_cc
            #cc2 = i.upper_cc
            #myscreen.addActor( camvtk.Sphere(center=(cc1.x,cc1.y,cc1.z),radius=0.005, color=camvtk.pink ) )
            #myscreen.addActor( camvtk.Sphere(center=(cc2.x,cc2.y,cc2.z),radius=0.005, color=camvtk.pink ) )

def yfiber(yvals,t,zh,myscreen,cutter,color):
    for y in yvals:
        f1 = ocl.Point(-0.5,y,zh) # start point of fiber
        f2 = ocl.Point(1.5,y,zh)  # end point of fiber
        f =  ocl.Fiber( f1, f2)
        i = ocl.Interval()
        #cutter.vertexPush(f,i,t)
        #cutter.facetPush(f,i,t)
        #cutter.edgePush(f,i,t)
        cutter.pushCutter(f,i,t)
        f.addInterval(i)
        drawFiber_clpts(myscreen, f, color)

def xfiber(xvals,t,zh,myscreen,cutter,color):
    for x in xvals:
        f1 = ocl.Point(x,-0.5,zh) # start point of fiber
        f2 = ocl.Point(x,1.5,zh)  # end point of fiber
        f =  ocl.Fiber( f1, f2)
        i = ocl.Interval()
        #cutter.vertexPush(f,i,t)
        #cutter.facetPush(f,i,t)
        #cutter.edgePush(f,i,t)
        cutter.pushCutter(f,i,t)
        f.addInterval(i)
        drawFiber_clpts(myscreen, f, color)

def oneCutterWaterline(myscreen, cutter, zh, color):
    fiber_range=2
    Nmax = 100
    yvals = [float(n-float(Nmax)/2)/Nmax*fiber_range for n in range(0,Nmax+1)]
    xvals = [float(n-float(Nmax)/2)/Nmax*fiber_range for n in range(0,Nmax+1)]
    yfiber(yvals,t,zh,myscreen,cutter, color)
    xfiber(xvals,t,zh,myscreen,cutter, color)

if __name__ == "__main__":  
    print(ocl.version())
    myscreen = camvtk.VTKScreen()
    myscreen.camera.SetPosition(0.5, 3, 2)
    myscreen.camera.SetFocalPoint(0.5, 0.5, 0)
    camvtk.drawArrows(myscreen,center=(-0.5,-0.5,-0.5))
    camvtk.drawOCLtext(myscreen)
    
    a = ocl.Point(0,1,0.2)
    b = ocl.Point(1,0.5,0.0)    
    c = ocl.Point(0.1,0.1,0.0)
    myscreen.addActor(camvtk.Point(center=(a.x,a.y,a.z), color=(1,0,1)))
    myscreen.addActor(camvtk.Point(center=(b.x,b.y,b.z), color=(1,0,1)))
    myscreen.addActor(camvtk.Point(center=(c.x,c.y,c.z), color=(1,0,1)))
    myscreen.addActor( camvtk.Line(p1=(a.x,a.y,a.z),p2=(c.x,c.y,c.z)) )
    myscreen.addActor( camvtk.Line(p1=(c.x,c.y,c.z),p2=(b.x,b.y,b.z)) )
    myscreen.addActor( camvtk.Line(p1=(a.x,a.y,a.z),p2=(b.x,b.y,b.z)) )
    t = ocl.Triangle(b,c,a)
    angle = math.pi/4
    diameter=0.3
    length=5
    cutter1 = ocl.BallCutter(diameter, length)
    cutter2 = ocl.CylCutter(diameter, length)
    cutter3 = ocl.BullCutter(diameter, diameter/4, length)
    cutter4 = ocl.ConeCutter(diameter, angle, length)
    #cutter = cutter.offsetCutter( 0.1 )
    
    
    fiber_range=2
    Nmax = 50
    yvals = [float(n-float(Nmax)/2)/Nmax*fiber_range for n in range(0,Nmax+1)]
    xvals = [float(n-float(Nmax)/2)/Nmax*fiber_range for n in range(0,Nmax+1)]
    zmin = -0.1
    zmax = 0.25
    zNmax =5
    dz = (zmax-zmin)/(zNmax-1)
    zvals=[]
    for n in range(0,zNmax):
        zvals.append(zmin+n*dz)

    for zh in zvals:
        oneCutterWaterline(myscreen, cutter1, zh, camvtk.yellow)
        oneCutterWaterline(myscreen, cutter2, zh, camvtk.cyan)
        oneCutterWaterline(myscreen, cutter3, zh, camvtk.mag)
        oneCutterWaterline(myscreen, cutter4, zh, camvtk.mag)
        #yfiber(yvals,t,zh,myscreen)
        #xfiber(xvals,t,zh,myscreen)
    
    print("done.")
    
    myscreen.render()    
    #w2if = vtk.vtkWindowToImageFilter()
    #w2if.SetInput(myscreen.renWin)
    #lwr = vtk.vtkPNGWriter()
    #lwr.SetInput( w2if.GetOutput() )

    myscreen.iren.Start()
    #raw_input("Press Enter to terminate") 
