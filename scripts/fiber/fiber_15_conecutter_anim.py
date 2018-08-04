import ocl
import camvtk
import time
import vtk
import datetime
import math
import os

def drawFiber_clpts(myscreen, f, fibercolor=camvtk.red):
    inter = f.getInts()
    #print "fiber has ", len(inter) , " intervals"
    for i in inter:
        if not i.empty():
            ip1 = f.point( i.lower )
            ip2 = f.point( i.upper )
            #myscreen.addActor( camvtk.Line(p1=(ip1.x,ip1.y,ip1.z),p2=(ip2.x,ip2.y,ip2.z), color=fibercolor) )
            myscreen.addActor( camvtk.Sphere(center=(ip1.x,ip1.y,ip1.z),radius=0.005, color=camvtk.clColor( i.lower_cc) ) )
            myscreen.addActor( camvtk.Sphere(center=(ip2.x,ip2.y,ip2.z),radius=0.005, color=camvtk.clColor( i.upper_cc) ) )
            #cc1 = i.lower_cc
            #cc2 = i.upper_cc
            #myscreen.addActor( camvtk.Sphere(center=(cc1.x,cc1.y,cc1.z),radius=0.005, color=camvtk.pink ) )
            #myscreen.addActor( camvtk.Sphere(center=(cc2.x,cc2.y,cc2.z),radius=0.005, color=camvtk.pink ) )

# colors from camvtk.py:
# ocl.CCType.VERTEX:         col = red
# ocl.CCType.EDGE_HORIZ:     col = orange
# ocl.CCType.EDGE_SHAFT:     col = mag

def yfiber(cutter,yvals,t,zh,myscreen):
    for y in yvals:
        f1 = ocl.Point(-5.5,y,zh) # start point of fiber
        f2 = ocl.Point(5.5,y,zh)  # end point of fiber
        f =  ocl.Fiber( f1, f2)
        i = ocl.Interval()
        cutter.pushCutter(f,i,t)
        f.addInterval(i)
        drawFiber_clpts(myscreen, f, camvtk.red)

def xfiber(cutter,xvals,t,zh,myscreen):
    for x in xvals:
        f1 = ocl.Point(x,-5.5,zh) # start point of fiber
        f2 = ocl.Point(x,5.5,zh)  # end point of fiber
        f =  ocl.Fiber( f1, f2)
        i = ocl.Interval()
        cutter.pushCutter(f,i,t)
        f.addInterval(i)
        drawFiber_clpts(myscreen, f, camvtk.lblue)

def drawScreen(a,b,c,filename,write_flag):  
    print ocl.version()
    myscreen = camvtk.VTKScreen()
    z_hi = a.z
    if b.z > z_hi:
        z_hi = b.z
    if c.z > z_hi:
        z_hi = c.z

    z_lo = a.z
    if b.z < z_lo:
        z_lo = b.z
    if c.z < z_lo:
        z_lo = c.z
    #z_hi = 0.3 # this is the shallow case
    #ztri = 0.8 # this produces the steep case where we hit the circular rim
    
    #z_lo = 0.1
    #a = ocl.Point(0,1,ztri)
    #b = ocl.Point(1,0.5,ztri)    
    #c = ocl.Point(0.2,0.2,ztri_lo)
    
    myscreen.addActor(camvtk.Point(center=(a.x,a.y,a.z), color=(1,0,1)))
    myscreen.addActor(camvtk.Point(center=(b.x,b.y,b.z), color=(1,0,1)))
    myscreen.addActor(camvtk.Point(center=(c.x,c.y,c.z), color=(1,0,1)))
    myscreen.addActor( camvtk.Line(p1=(a.x,a.y,a.z),p2=(c.x,c.y,c.z)) )
    myscreen.addActor( camvtk.Line(p1=(c.x,c.y,c.z),p2=(b.x,b.y,b.z)) )
    myscreen.addActor( camvtk.Line(p1=(a.x,a.y,a.z),p2=(b.x,b.y,b.z)) )
    t = ocl.Triangle(b,c,a)
    angle = math.pi/5
    diameter=0.3
    length=5
    #cutter = ocl.BallCutter(diameter, length)
    #cutter = ocl.CylCutter(diameter, length)
    #cutter = ocl.BullCutter(diameter, diameter/4, length)
    cutter = ocl.ConeCutter(diameter, angle, length)
    #cutter = cutter.offsetCutter( 0.1 )
    
    print "cutter= ", cutter
    print "length=", cutter.getLength()
    print "fiber..."
    range=2
    Nmax = 100
    yvals = [float(n-float(Nmax)/2)/Nmax*range for n in xrange(0,Nmax+1)]
    xvals = [float(n-float(Nmax)/2)/Nmax*range for n in xrange(0,Nmax+1)]
    zmin = z_lo - 0.3
    zmax = z_hi
    zNmax = 20
    dz = (zmax-zmin)/(zNmax-1)
    zvals=[]
    for n in xrange(0,zNmax):
        zvals.append(zmin+n*dz)
    for zh in zvals:
        yfiber(cutter,yvals,t,zh,myscreen)
        xfiber(cutter,xvals,t,zh,myscreen)
    print "done."
    myscreen.camera.SetPosition(-2, -1, 3)
    myscreen.camera.SetFocalPoint(1.0, 0.0, -0.5)
    camvtk.drawArrows(myscreen,center=(-0.5,-0.5,-0.5))
    camvtk.drawOCLtext(myscreen)
    myscreen.render()    
    """
    w2if = vtk.vtkWindowToImageFilter()
    w2if.SetInput(myscreen.renWin)
    lwr = vtk.vtkPNGWriter()
    lwr.SetInput( w2if.GetOutput() )
    w2if.Modified()
    lwr.SetFileName(filename)
    if write_flag:
        lwr.Write()
        print "wrote ",filename
    """
    
    #myscreen.iren.Start()
    #raw_input("Press Enter to terminate") 

if __name__ == "__main__":  
    ztri = 0.3 # this is the shallow case
    #ztri = 0.8 # this produces the steep case where we hit the circular rim
    ztri_lo = 0.1
    Nmax = 300
    thetamax = 2*math.pi
    for n in xrange(0,Nmax):
        theta=thetamax/Nmax
        a = ocl.Point(0,1,ztri)
        a.xRotate(theta*n)
        b = ocl.Point(1,0.0,0)    
        b.xRotate(theta*n)
        c = ocl.Point(0.2,0.0,ztri)
        c.xRotate(theta*n)
        current_dir = os.getcwd()
        filename = current_dir + "/frames/conecutter_"+ ('%05d' % n)+".png"
        drawScreen(a,b,c,filename, 1)

