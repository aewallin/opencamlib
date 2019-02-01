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
magenta =(float(255)/255,0,float(225)/255)
pink =(float(255)/255,float(193)/255,float(203)/255)

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
    length = 5
    cutter = cam.BullCutter(1,0.2,length)

    
    print(cutter)
    
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
    tor.SetSurface()
    tor.SetOpacity(0.3)
    myscreen.addActor(tor)
    
    cyl = camvtk.Cylinder(center=(cl.x,cl.y,cl.z) , radius=radius1, height=2, color=(0,1,1),
                    rotXYZ=(90,0,0), resolution=50 )
    #myscreen.addActor(cyl)

    tube = camvtk.Tube(p1=(a.x,a.y,a.z),p2=(b.x,b.y,b.z),radius=radius2, color=(1,1,0))
    tube.SetOpacity(0.1)
    myscreen.addActor(tube)
    
    cir= camvtk.Circle(radius=radius1, center=(cl.x,cl.y,cl.z), color=yellow)
    myscreen.addActor(cir)
    
    clp = camvtk.Sphere(radius=0.02, center=(cl.x,cl.y,cl.z), color=yellow)
    myscreen.addActor(clp)
    
    # short axis of ellipse = radius2
    # long axis of ellipse = radius2/sin(theta)
    # where theta is the slope of the line
    dx = b.x - a.x
    dz = b.z - a.z
    print("dx=", dx)
    print("dz=", dz)
    theta = math.atan(dz/dx)
    print("theta=",theta)
    a = abs( radius2/math.sin(theta) )
    print("a=", a, " = ", a/radius2,"* radius2")
    # ellipse
    #a=2
    b=radius2
    print("b= ", b)
    
    
    ecen_tmp=cam.Point(1.38,2,0)
    resolution=50
    for n in range(0,resolution):
        angle1= (float(n)/float(resolution))*2*math.pi
        angle2= (float(n+1)/float(resolution))*2*math.pi
        x=ecen_tmp.x + a*math.cos(angle1)
        y=ecen_tmp.y + b*math.sin(angle1)
        x2=ecen_tmp.x + a*math.cos(angle2)
        y2=ecen_tmp.y + b*math.sin(angle2)
        
        #myscreen.addActor(camvtk.Point(center=(x,y,0), color=(1,0,1)))
        #myscreen.addActor( camvtk.Line(p1=(x,y,0),p2=(x2,y2,0)) )
    
    oe = cam.Ellipse(ecen_tmp, a, b, radius1)
    
    myscreen.camera.SetPosition(5, 7, 1)
    myscreen.camera.SetFocalPoint(0.5, 0.5, 0)
        
    nmax=80
    dd = float(4.0)/nmax
    diangles = [ n*dd for n in range(nmax) ]
    epos1 = cam.EllipsePosition()
    epos2 = cam.EllipsePosition()
    for n in range(nmax):
        #s = float(n)/float(nmax-1) * 2-1
        #t = oe.teval(s, 1)
        #t2 = oe.teval(s, 0)
        n2 = n+1
        if n2==nmax:
            n2=0
        epos1.setDiangle( diangles[n] )
        epos2.setDiangle( diangles[n2] )
        
        p1 = oe.ePoint( epos1 )
        p2 = oe.ePoint( epos2 )
        
        p1o = oe.oePoint( epos1 )
        p2o = oe.oePoint( epos2 )
        #print "s=", s, "t=", t," epoint=", p1.str()
        
        myscreen.addActor( camvtk.Line(p1=(p1.x,p1.y,p1.z),p2=(p2.x,p2.y,p2.z), color=magenta) )
        
        myscreen.addActor( camvtk.Line(p1=(p1o.x,p1o.y,p1o.z),p2=(p2o.x,p2o.y,p2o.z), color=pink) )
        #myscreen.addActor(camvtk.Point(center=(p1.x,p1.y,0), color=green))
        #myscreen.addActor(camvtk.Point(center=(p2.x,p2.y,0), color=red))
        
        #myscreen.addActor(camvtk.Point(center=(p1o.x,p1o.y,0), color=green))
        #myscreen.addActor(camvtk.Point(center=(p2o.x,p2o.y,0), color=red))
    
    # ellipse point, normal
    #epos1.setDiangle(3.48) # approx solution
    epos1.setDiangle(0)
    ep = oe.ePoint( epos1 )
    oep = oe.oePoint( epos1 )
    myscreen.addActor( camvtk.Sphere(radius=0.02, center=(ep.x,ep.y,ep.z), color=magenta) )
    myscreen.addActor( camvtk.Sphere(radius=0.02, center=(oep.x,oep.y,oep.z), color=pink) )
    myscreen.addActor( camvtk.Line(p1=(ep.x,ep.y,ep.z),p2=(oep.x,oep.y,oep.z), color=red) )
    
    myscreen.render()
    #time.sleep(0.5)
     
    print("rendering...",)
    #for cl,cc in zip(clpoints,ccpoints):
    #    myscreen.addActor( camvtk.Point(center=(cl.x,cl.y,cl.z) , color=ccColor(cc) ) )
    #    if cc.type != cam.CCType.NONE: # only render interesting cc-points
    #        myscreen.addActor( camvtk.Point(center=(cc.x,cc.y,cc.z) , color=ccColor2(cc) ) )

                        
    print("done.")
    

    

    myscreen.render()
    
    #w2if = vtk.vtkWindowToImageFilter()
    #w2if.SetInput(myscreen.renWin)
    #lwr = vtk.vtkPNGWriter()
    #lwr.SetInput( w2if.GetOutput() )
    #w2if.Modified()
    #lwr.SetFileName("5_all.png")
    #lwr.Write()



    myscreen.iren.Start()
    #raw_input("Press Enter to terminate") 
    
