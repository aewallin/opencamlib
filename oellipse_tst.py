import ocl as cam
import camvtk
import time
import vtk
import math

red= (1,0,0)
green= (0,1,0)
blue= (0,0,1)
cyan=  (0,1,1)
yellow= (1,1,0)

def ccColor2(cc):
    """ this function returns a different color depending on the type of
        the CC-point. Useful for visualizing CL or CC points """
    if cc.type==cam.CCType.FACET:
        col = (1,0,1)
    elif cc.type == cam.CCType.VERTEX:
        col = (1,1,0)
    elif cc.type == cam.CCType.EDGE:
        col = (0,1,1)
    elif cc.type == cam.CCType.NONE:
        col = (1,1,1)  
    elif cc.type == cam.CCType.ERROR:
        col = (0,0.5,1)
    return col    

class OffsetEllipse():
    """
    // (s, t) where:  s^2 + t^2 = 1
    // point of ellipse is:  ecen + j s + n t
    // tangent at point is:  -j t + n s
    // normal at point is:  j (s / eccen) + n (t * eccen)
    // point on offset-ellipse:  point on ellipse + offrad*normal
    """
    def __init__(self, ecen, a, b, ofs):
        self.a = a
        self.b = b
        self.ecen = ecen
        self.ofs = ofs
        
    def ePoint(self,t,s):
        # return a point on the ellipse
        p = cam.Point()
        p.x = self.ecen.x + self.a*s
        p.y = self.ecen.y + self.b*t
        return p
    
    def oePoint(self,t,s):
        # return point on offset-ellipse
        p = self.ePoint(t,s)
        normal = cam.Point( self.b*s, self.a*t, 0)
        normal.normalize()
        p = p + self.ofs*normal
        return p
    
    def teval(self, s, side):
        ssq = s*s
        tsq = 1 - ssq
        if side == 1:
            return 1 * math.sqrt(tsq)
        else:
            return -1* math.sqrt(tsq)

if __name__ == "__main__":  
    myscreen = camvtk.VTKScreen()
    
    a=cam.Point(3,2,-2)
    b=cam.Point(-1,2,3)    
    
    myscreen.addActor(camvtk.Point(center=(a.x,a.y,a.z), color=(1,0,1)));
    myscreen.addActor(camvtk.Point(center=(b.x,b.y,b.z), color=(1,0,1)));
    #c=cam.Point(0,0,0.3)
    myscreen.addActor( camvtk.Line(p1=(a.x,a.y,a.z),p2=(b.x,b.y,b.z)) )
    #t = cam.Triangle(a,b,c)
    
    cutter = cam.BullCutter(1,0.2)

    
    print cutter.str()
    
    xar = camvtk.Arrow(color=red, rotXYZ=(0,0,0))
    myscreen.addActor(xar)
    yar = camvtk.Arrow(color=green, rotXYZ=(0,0,90))
    myscreen.addActor(yar)
    zar = camvtk.Arrow(color=blue, rotXYZ=(0,-90,0))
    myscreen.addActor(zar)
    
    cl = cam.Point(2.193, 1, 0)
    radius1=1
    radius2=0.25
    
    tor = camvtk.Toroid(r1=radius1, r2=radius2, center=(cl.x, cl.y, cl.z),rotXYZ=(0,0,0))
    #tor.SetWireframe()
    #myscreen.addActor(tor)
    
    cyl = camvtk.Cylinder(center=(cl.x,cl.y,cl.z) , radius=radius1, height=2, color=(0,1,1),
                    rotXYZ=(90,0,0), resolution=50 )
    #myscreen.addActor(cyl)

    tube = camvtk.Tube(p1=(a.x,a.y,a.z),p2=(b.x,b.y,b.z),color=(1,1,0))
    tube.SetOpacity(0.2)
    myscreen.addActor(tube)
    
    cir= camvtk.Circle(radius=radius1, center=(cl.x,cl.y,cl.z), color=yellow)
    myscreen.addActor(cir)
    
    clp = camvtk.Point(center=(cl.x,cl.y,cl.z))
    myscreen.addActor(clp)
    
    # short axis of ellipse = radius2
    # long axis of ellipse = radius2/sin(theta)
    # where theta is the slope of the line
    dx = b.x - a.x
    dz = b.z - a.z
    print "dx=", dx
    print "dz=", dz
    theta = math.atan(dz/dx)
    print "theta=",theta
    a = abs( radius2/math.sin(theta) )
    print "a=", a, " = ", a/radius2,"* radius2"
    # ellipse
    #a=2
    b=radius2
    print "b= ", b
    
    
    ecen_tmp=cam.Point(1.38,2,0)
    resolution=50
    for n in xrange(0,resolution):
        angle1= (float(n)/float(resolution))*2*math.pi
        angle2= (float(n+1)/float(resolution))*2*math.pi
        x=ecen_tmp.x + a*math.cos(angle1)
        y=ecen_tmp.y + b*math.sin(angle1)
        x2=ecen_tmp.x + a*math.cos(angle2)
        y2=ecen_tmp.y + b*math.sin(angle2)
        
        #myscreen.addActor(camvtk.Point(center=(x,y,0), color=(1,0,1)))
        #myscreen.addActor( camvtk.Line(p1=(x,y,0),p2=(x2,y2,0)) )
    
    oe = OffsetEllipse(ecen_tmp, a, b, radius1)
    
    myscreen.camera.SetPosition(5, 7, 1)
    myscreen.camera.SetFocalPoint(0.5, 0.5, 0)
        
    nmax=80
    for n in xrange(0,nmax):
        s = float(n)/float(nmax-1) * 2-1
        t = oe.teval(s, 1)
        t2 = oe.teval(s, 0)
        p1 = oe.ePoint(s,t)
        p2 = oe.ePoint(s,t2)
        p1o = oe.oePoint(s,t)
        p2o = oe.oePoint(s,t2)
        #print "s=", s, "t=", t," epoint=", p1.str()
        myscreen.addActor(camvtk.Point(center=(p1.x,p1.y,0), color=green))
        myscreen.addActor(camvtk.Point(center=(p2.x,p2.y,0), color=red))
        myscreen.addActor(camvtk.Point(center=(p1o.x,p1o.y,0), color=green))
        myscreen.addActor(camvtk.Point(center=(p2o.x,p2o.y,0), color=red))
        myscreen.render()
        time.sleep(0.5)
     
    print "rendering...",
    #for cl,cc in zip(clpoints,ccpoints):
    #    myscreen.addActor( camvtk.Point(center=(cl.x,cl.y,cl.z) , color=ccColor(cc) ) )
    #    if cc.type != cam.CCType.NONE: # only render interesting cc-points
    #        myscreen.addActor( camvtk.Point(center=(cc.x,cc.y,cc.z) , color=ccColor2(cc) ) )

                        
    print "done."
    

    

    myscreen.render()
    
    w2if = vtk.vtkWindowToImageFilter()
    w2if.SetInput(myscreen.renWin)
    lwr = vtk.vtkPNGWriter()
    lwr.SetInput( w2if.GetOutput() )
    w2if.Modified()
    lwr.SetFileName("5_all.png")
    #lwr.Write()



    myscreen.iren.Start()
    #raw_input("Press Enter to terminate") 
    
