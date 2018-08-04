import ocl
import camvtk
import time
import vtk
import datetime
import math

def drawFiber_clpts(myscreen, f, fibercolor=camvtk.red):
    inter = f.getInts()
    #print "fiber has ", len(inter) , " intervals"
    for i in inter:
        if not i.empty():
            ip1 = f.point( i.lower )
            ip2 = f.point( i.upper )
            ballradius = 0.002
            ballradius = 0.005
            #myscreen.addActor( camvtk.Line(p1=(ip1.x,ip1.y,ip1.z),p2=(ip2.x,ip2.y,ip2.z), color=fibercolor) )
            myscreen.addActor( camvtk.Sphere(center=(ip1.x,ip1.y,ip1.z),radius=ballradius, color=camvtk.clColor( i.lower_cc) ) )
            myscreen.addActor( camvtk.Sphere(center=(ip2.x,ip2.y,ip2.z),radius=ballradius, color=camvtk.clColor( i.upper_cc) ) )
            
            #cc1 = i.lower_cc
            #cc2 = i.upper_cc
            #myscreen.addActor( camvtk.Sphere(center=(cc1.x,cc1.y,cc1.z),radius=0.005, color=camvtk.pink ) )
            #myscreen.addActor( camvtk.Sphere(center=(cc2.x,cc2.y,cc2.z),radius=0.005, color=camvtk.pink ) )

# colors from camvtk.py:
# ocl.CCType.VERTEX:         col = red
# ocl.CCType.EDGE_HORIZ:     col = orange
# ocl.CCType.EDGE_SHAFT:     col = mag

def yfiber(yvals,t,zh,myscreen):
    for y in yvals:
        f1 = ocl.Point(-3.5,y,zh) # start point of fiber
        f2 = ocl.Point(3.5,y,zh)  # end point of fiber
        f =  ocl.Fiber( f1, f2)
        i = ocl.Interval()
        cutter.pushCutter(f,i,t)
        f.addInterval(i)
        drawFiber_clpts(myscreen, f, camvtk.red)

def xfiber(xvals,t,zh,myscreen):
    for x in xvals:
        f1 = ocl.Point(x,-15.5,zh) # start point of fiber
        f2 = ocl.Point(x,55.5,zh)  # end point of fiber
        f =  ocl.Fiber( f1, f2)
        i = ocl.Interval()
        cutter.pushCutter(f,i,t)
        f.addInterval(i)
        drawFiber_clpts(myscreen, f, camvtk.lblue)

if __name__ == "__main__":  
    print ocl.version()
    myscreen = camvtk.VTKScreen()
    
    ztri = 0.3 # this is the shallow case
    #ztri = 0.8 # this produces the steep case where we hit the circular rim
    
    ztri_lo = 0.1
    a = ocl.Point(0,1,ztri)
    b = ocl.Point(1,0.5,ztri)    
    c = ocl.Point(0.2,0.2,ztri_lo)
    
    a = ocl.Point(0,1,0.3)
    b = ocl.Point(1,0.5,0.3)    
    c = ocl.Point(0,0,0)
    
    myscreen.addActor(camvtk.Point(center=(a.x,a.y,a.z), color=(1,0,1)))
    myscreen.addActor(camvtk.Point(center=(b.x,b.y,b.z), color=(1,0,1)))
    myscreen.addActor(camvtk.Point(center=(c.x,c.y,c.z), color=(1,0,1)))
    myscreen.addActor( camvtk.Line(p1=(a.x,a.y,a.z),p2=(c.x,c.y,c.z)) )
    myscreen.addActor( camvtk.Line(p1=(c.x,c.y,c.z),p2=(b.x,b.y,b.z)) )
    myscreen.addActor( camvtk.Line(p1=(a.x,a.y,a.z),p2=(b.x,b.y,b.z)) )
    t = ocl.Triangle(b,c,a)
    angle = math.pi/5
    diameter=0.6
    length=25
    cutter = ocl.BallCutter(diameter, length)
    #cutter = ocl.CylCutter(diameter, length)
    #cutter = ocl.BullCutter(diameter, diameter/4, length)
    #cutter = ocl.ConeCutter(diameter, angle, length)
    #cutter = cutter.offsetCutter( 0.1 )
    
    #cutter = ocl.CompCylCutter(diameter, length)
    #cutter = ocl.CompBallCutter(diameter, length)
    
    #cutter =  ocl.CylConeCutter(diameter/float(3), diameter, math.pi/float(9))
    cutter =  ocl.BallConeCutter(diameter/float(3), diameter, math.pi/float(5))
    
    
    print "cutter= ", cutter
    print "length=", cutter.getLength()
    print "fiber..."
    fiberrange=5
    Nmax = 100
    yvals = [float(n-float(Nmax)/2)/Nmax*fiberrange for n in xrange(0,Nmax+1)]
    xvals = [float(n-float(Nmax)/2)/Nmax*fiberrange for n in xrange(0,Nmax+1)]
    """
    zmin = ztri_lo - 0.2
    zmax = ztri
    zNmax = 20
    dz = (zmax-zmin)/(zNmax-1)
    zvals=[]
    for n in xrange(0,zNmax):
        zvals.append(zmin+n*dz)
    """
    
    zheights=[]
    Nmax=30
    zmin=-0.85
    zmax= 0.3
    dz = (zmax-zmin)/float(Nmax-1)
    z = zmin
    for n in xrange(Nmax):
        zheights.append(z)
        z=z+dz
    #zheights= []
    #zheights.append(-0.25)
    
    print len(zheights)," to do: ",
    n=0
    for zh in zheights:
        print n,
        yfiber(yvals,t,zh,myscreen)
        xfiber(xvals,t,zh,myscreen)
        n=n+1
    print "done."
    myscreen.camera.SetPosition(-1, -1, 3)
    myscreen.camera.SetFocalPoint(0.5, 0.5, 0)
    camvtk.drawArrows(myscreen,center=(-0.5,-0.5,-0.5))
    camvtk.drawOCLtext(myscreen)
    myscreen.render()    
    #w2if = vtk.vtkWindowToImageFilter()
    #w2if.SetInput(myscreen.renWin)
    #lwr = vtk.vtkPNGWriter()
    #lwr.SetInput( w2if.GetOutput() )

    myscreen.iren.Start()
    #raw_input("Press Enter to terminate") 
