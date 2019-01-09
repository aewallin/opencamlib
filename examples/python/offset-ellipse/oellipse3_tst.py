import ocl as cam
import camvtk
import time
import vtk
import math
import datetime

red= (1,0,0)
green= (0,1,0)
blue= (0,0,1)
cyan=  (0,1,1)
yellow= (1,1,0)
pink = ( float(255)/255,float(192)/255,float(203)/255)
grey = ( float(127)/255,float(127)/255,float(127)/255)
orange = ( float(255)/255,float(165)/255,float(0)/255)

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
        
    def ePoint(self,epos):
        # return a point on the ellipse
        p = cam.Point()
        p.x = self.ecen.x + self.a*epos.s
        p.y = self.ecen.y + self.b*epos.t
        return p
    
    def oePoint(self,epos):
        # return point on offset-ellipse
        p = self.ePoint(epos)
        normal = self.eNorm(epos)
        p.x = p.x + self.ofs*normal.x
        p.y = p.y + self.ofs*normal.y
        return p
    
    def eNorm(self, epos):
        normal = cam.Point( self.b*epos.s, self.a*epos.t, 0)
        normal.normalize()
        return normal
        
    def eTang(self, epos):
        p = cam.Point(-self.a*epos.t, self.b*epos.s, 0)
        p.normalize()
        return p
    
    def error(self,epos,cl):
        p1 = self.oePoint(epos)
        p2 = cl
        dx = p1.x - cl.x
        dy = p1.y - cl.y
        #return dx*dx+dy*dy
        return dy
            


class EPos():
    """
    // (s, t) where:  s^2 + t^2 = 1
    // point of ellipse is:  ecen + j s + n t
    // tangent at point is:  -j t + n s
    // normal at point is:  j (s / eccen) + n (t * eccen)
    // point on offset-ellipse:  point on ellipse + offrad*normal
    """
    def __init__(self):
        self.t = 1
        self.s = 0
        self.sett(self.t,1)
        
    def sets(self, s, side):
        if s > 1.0:
            s = 1.0
        if s < -1.0:
            s = -1.0
            
        self.s = s
        ssq = s*s
        tsq = 1 - ssq
        if side == 1:
            self.t =  1 * math.sqrt(tsq)
        else:
            self.t = -1 * math.sqrt(tsq)
            
    def sett(self, t, side):
        if t > 1.0:
            t = 1.0
        if t < -1.0:
            t = -1.0
            
        self.t = t
        tsq = t*t
        ssq = 1 - tsq
        if side == 1:
            self.s = 1 * math.sqrt(ssq)
        else:
            self.s = -1* math.sqrt(ssq)
                
    def stepTang(self, ellipse, delta):
        tang = oe.eTang(self)
        #print "         epos= (", self.s," , ", self.t , " )" 
        #print "steptang tang=", tang.str()
        if abs(tang.x) > abs(tang.y):
            #print "s-dir step"
            news = self.s + delta*tang.x
            if self.t > 0:
                self.sets(news,1)
            else:
                self.sets(news,0)
        else:
            #print "t-dir step"
            newt = self.t + delta*tang.y
            if self.s>0:
                self.sett( newt,1)
            else:
                self.sett( newt,0)

def solver(oe, epos, cl):
    endcondition = 0
    NRStep = 0.1
    n=0
    while not endcondition:
        current_error = oe.error(epos, cl)
        #print "current error=", current_error
        epos_tmp = EPos()
        epos_tmp.s = epos.s
        epos_tmp.t = epos.t
        # take a small step, to determine rerivative:
        dt = 0.2*NRStep
        epos_tmp.stepTang(oe,dt)
        new_error = oe.error(epos_tmp, cl)
        #print "new_error=", new_error
        deriv = (new_error-current_error)/dt
        #print "derivative = ", deriv
        # take Newton rhapson step
        NRStep = (-current_error/deriv)
        #print " NRstep=", NRStep
        #NRStep=0.05 # debug/demo
        epos5.stepTang(oe, NRStep)
        
        # check endcondition    
        if abs(current_error) < 1e-8:
            endcondition=1
        if n>125:
            endcondition=1
        n=n+1
    return n
        
if __name__ == "__main__":  
    myscreen = camvtk.VTKScreen()
    
    myscreen.camera.SetPosition(5, 3, 2)
    myscreen.camera.SetFocalPoint(1.38,1, 0)
    
    a=cam.Point(3,2,-2)
    b=cam.Point(-1,2,3)    
    
    myscreen.addActor(camvtk.Point(center=(a.x,a.y,a.z), color=(1,0,1)));
    myscreen.addActor(camvtk.Point(center=(b.x,b.y,b.z), color=(1,0,1)));
    #c=cam.Point(0,0,0.3)
    myscreen.addActor( camvtk.Line(p1=(a.x,a.y,a.z),p2=(b.x,b.y,b.z)) )
    #t = cam.Triangle(a,b,c)
    
    cutter_length = 2
    cutter = cam.BullCutter(1,0.2, cutter_length)

    
    print cutter
    
    xar = camvtk.Arrow(color=red, rotXYZ=(0,0,0))
    #xar.SetFlat()
    myscreen.addActor(xar)
    yar = camvtk.Arrow(color=green, rotXYZ=(0,0,90))
    #yar.SetGouraud()
    myscreen.addActor(yar)
    zar = camvtk.Arrow(color=blue, rotXYZ=(0,-90,0))
    #zar.SetPhong()
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
    #myscreen.addActor(tube)
    
    cir= camvtk.Circle(radius=radius1, center=(cl.x,cl.y,cl.z), color=yellow)
    myscreen.addActor(cir)
    
    clp = camvtk.Point(center=(cl.x,cl.y,cl.z))
    myscreen.addActor(clp)
    
    # short axis of ellipse = radius2
    # long axis of ellipse = radius2/sin(theta)
    # where theta is the slope of the line
    dx = b.x - a.x
    dz = b.z - a.z
    #print "dx=", dx
    #print "dz=", dz
    theta = math.atan(dz/dx)
    #print "theta=",theta
    a = abs( radius2/math.sin(theta) )
    #print "a=", a, " = ", a/radius2,"* radius2"
    # ellipse
    #a=2
    b=radius2
    #print "b= ", b
    
    
    ecen_tmp=cam.Point(1.38,2,0)
    resolution=50
    """
    for n in xrange(0,resolution):
        angle1= (float(n)/float(resolution))*2*math.pi
        angle2= (float(n+1)/float(resolution))*2*math.pi
        x=ecen_tmp.x + a*math.cos(angle1)
        y=ecen_tmp.y + b*math.sin(angle1)
        x2=ecen_tmp.x + a*math.cos(angle2)
        y2=ecen_tmp.y + b*math.sin(angle2)
        
        #myscreen.addActor(camvtk.Point(center=(x,y,0), color=(1,0,1)))
        #myscreen.addActor( camvtk.Line(p1=(x,y,0),p2=(x2,y2,0)) )
    """
    
    oe = OffsetEllipse(ecen_tmp, a, b, radius1)
    oe2 = OffsetEllipse(ecen_tmp, a, b, 0.05)

        
    nmax=20
    delta=0.05
    td = 1
    epos1 = EPos() 
    epos2 = EPos() 
    epos3 = EPos()
    epos4 = EPos()
    epos5 = EPos()
    """
    for n in xrange(0,nmax):
        s = float(n)/float(nmax-1) * float(2)/math.sqrt(2) - float(1)/math.sqrt(2)
        t = float(n)/float(nmax-1) * float(2)/math.sqrt(2) - float(1)/math.sqrt(2)
        
        epos1.sets(s,1)
        epos2.sets(s,0)
        epos3.sett(t,1)
        epos4.sett(t,0)

        p1 = oe.ePoint(epos1)
        p2 = oe.ePoint(epos2)
        p3 = oe.ePoint(epos3)
        p4 = oe.ePoint(epos4)
        p5 = oe.ePoint(epos5)
        
        p1o = oe.oePoint(epos1)
        p2o = oe.oePoint(epos2)
        #print "s=", s, "t=", t," epoint=", p1.str()
        myscreen.addActor(camvtk.Point(center=(p1.x,p1.y,0), color=green)) # green steps along s   (side=1)
        myscreen.addActor(camvtk.Point(center=(p2.x,p2.y,0), color=red))   # red steps along s     (side=0)
        myscreen.addActor(camvtk.Point(center=(p3.x,p3.y,0), color=orange)) # orange steps along t (side=1)
        myscreen.addActor(camvtk.Point(center=(p4.x,p4.y,0), color=pink))  # pink steps along t    (side=0)
        
        #myscreen.addActor(camvtk.Sphere(center=(p5.x,p5.y,0), radius=0.02, color=red))
        #myscreen.addActor(camvtk.Point(center=(p1o.x,p1o.y,0), color=green))
        #myscreen.addActor(camvtk.Point(center=(p2o.x,p2o.y,0), color=red))
        
        
        tx = float(n)/float(nmax-1) * 2 - 1
        sx = oe.seval(tx, 1)
        sx2 = oe.seval(tx, 0)
        p3 = oe.ePoint(sx,tx)
        p4 = oe.ePoint(sx2,tx)
        #myscreen.addActor(camvtk.Point(center=(p3.x,p3.y,0), color=orange))
        #myscreen.addActor(camvtk.Point(center=(p4.x,p4.y,0), color=pink))
        
        sd = oe.seval(td,1)
        p5 = oe.ePoint(td,sd)
        myscreen.addActor(camvtk.Point(center=(p5.x,p5.y,0), color=orange))
        
        myscreen.render()
        #time.sleep(0.05)
    """
    
    t = camvtk.Text()
    t.SetPos( (myscreen.width-450, myscreen.height-30) )
    
    myscreen.addActor( t)
    t2 = camvtk.Text()
    t2.SetPos( (50, myscreen.height-150) )
    
    myscreen.addActor( t2)
        
    #epos5.sets(0.5,1)
    Nsteps=62
    endcondition = 0
    n = 1
    NRStep=0.1
    
    #w2if = vtk.vtkWindowToImageFilter()
    #w2if.SetInput(myscreen.renWin)
    #lwr = vtk.vtkPNGWriter()
    #lwr.SetInput( w2if.GetOutput() )
  
        
    epos5.sets(0,1)
    epos1.sets(0,1)
    #while not endcondition:
    convlist=[]
    for n in xrange(0,Nsteps):
        t.SetText("OpenCAMLib 10.03-beta, " + datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S"))
        #current_error = oe.error(epos5, cl)
        epos5.s=epos1.s
        epos5.t=epos1.t
        #print "current error=", current_error
        #epos_tmp = EPos()
        #epos_tmp.s = epos5.s
        #epos_tmp.t = epos5.t
        # take a small step, to determine rerivative:
        #dt = 0.2*NRStep
        #epos_tmp.stepTang(oe,dt)
        #new_error = oe.error(epos_tmp, cl)
        #print "new_error=", new_error
        #deriv = (new_error-current_error)/dt
        #print "derivative = ", deriv
        #paramtext = "(s, t) = (%3.3f, %3.3f)\n NR iteration # = %i \n error= %3.9f\n de=%3.3f" % (epos5.s, epos5.t, n, current_error,deriv)
        #t2.SetText(paramtext)
        # take Newton rhapson step
        #NRStep = (-current_error/deriv)
        #print " step=", NRStep
        #NRStep=0.05 # debug/demo
        #epos5.stepTang(oe, NRStep)
        
     
        
        
        #p6 = oe.eTang(epos5)
        #p7 = oe.oePoint(epos5)
        #oesphere = camvtk.Sphere(center=(p7.x,p7.y,p7.z), radius=0.02, color=green)
        #tangline = camvtk.Line(p1=(p5.x,p5.y,p5.z),p2=(p5.x+p6.x,p5.y+p6.y,p5.z+p6.z))
        #normline = camvtk.Line(p1=(p5.x,p5.y,p5.z),p2=(p7.x,p7.y,p7.z), color=yellow)
        
        #myscreen.addActor( tangline )
        #myscreen.addActor( normline )
        #myscreen.addActor( oesphere )
        p5 = oe.ePoint(epos5)
        pt = oe2.oePoint(epos5)
        #print "before= ", epos5.s, " , ", epos5.t
        nsteps = solver(oe, epos5, cl)
        print nsteps 
        convlist.append(nsteps)
        
        convtext = "%i" % (nsteps)
        print (pt.x, pt.y, pt.z)
        #center=(pt.x, pt.y, pt.z)    
        tst = camvtk.Text3D( color=(1,1,1), center=(pt.x, pt.y, 0)  , 
                            text=convtext, scale=0.02)
        #tst.SetCamera(myscreen.camera)
        myscreen.addActor(tst)
                
        colmax=11
        colmin=4
        nsteps = nsteps - colmin
        colmax = colmax - colmin
        convcolor=( float(nsteps*nsteps)/(colmax), float((colmax-nsteps))/colmax, 0 )
        esphere = camvtk.Sphere(center=(p5.x,p5.y,0), radius=0.02, color=convcolor)
        myscreen.addActor(esphere)
        

              
        
        #print "after= ", epos5.s, " , ", epos5.t
        epos1.stepTang(oe, 0.1)
        #print "after= ", epos5.s, " , ", epos5.t
        
        myscreen.render()
        #time.sleep(0.1)
        #raw_input("Press Enter to terminate") 
        #if abs(current_error) < 1e-8:
        #    endcondition=1
        #if n>125:
        #    endcondition=1
            
        #if not endcondition:
        #    myscreen.removeActor(esphere)
        #    myscreen.removeActor(tangline)
        #    myscreen.removeActor(normline)
        #    myscreen.removeActor(oesphere)
            

        #w2if.Modified()
        #lwr.SetFileName("5_all.png")
        """
        for i in xrange(0,10):
            lwr.SetFileName("frames/oe_nrx"+ ('%05d%02d' % (n,i))+".png")
            lwr.Write()
        """
        n=n+1
    print convlist
    #print "rendering...",
    #for cl,cc in zip(clpoints,ccpoints):
    #    myscreen.addActor( camvtk.Point(center=(cl.x,cl.y,cl.z) , color=ccColor(cc) ) )
    #    if cc.type != cam.CCType.NONE: # only render interesting cc-points
    #        myscreen.addActor( camvtk.Point(center=(cc.x,cc.y,cc.z) , color=ccColor2(cc) ) )

                        
    print "done."
    

    

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
    
