import ocl
import camvtk
import time
import vtk
import datetime
import math
import random


def drawVertex(myscreen, p, vertexColor, rad=1):
    myscreen.addActor( camvtk.Sphere( center=(p.x,p.y,p.z), radius=rad, color=vertexColor ) )

def drawEdge(myscreen, e, edgeColor=camvtk.yellow):
    p1 = e[0]
    p2 = e[1]
    myscreen.addActor( camvtk.Line( p1=( p1.x,p1.y,p1.z), p2=(p2.x,p2.y,p2.z), color=edgeColor ) )

def drawCircle(myscreen, c, circleColor):
    myscreen.addActor( camvtk.Circle( center=(c.c.x,c.c.y,c.c.z), radius=c.r, color=circleColor ) )



# CIRCLE def
# (x(t) - xc1)^2 + (y(t)-yc1)^2 = (r1+k1*t)^2 
class Circle:
    def __init__(self,c=ocl.Point(0,0),r=1,cw=1):
        self.c = c
        self.r = r
        self.cw = cw

# LINE def
# a1 x + b1 y + c1 + k1 t = 0 and a*a + b*b = 1 
class Line:
    def __init__(self,a,b,c):
        self.a=a
        self.b=b
        selc.c=c

class CircleCircle:
    # CIRCLE / CIRCLE
    # d= sqrt( square(xc1-xc2) + square(yc1-yc2) ) 
    # cw=-1 for CCW arc and +1 otherwise 
    def __init__(self, c1, c2):
        self.d     = (c1.c-c2.c).xyNorm() 
        self.alfa1 = 0.0
        self.alfa2 = 0.0
        self.alfa3 = 0.0
        self.alfa4 = 0.0            
        if ( self.d > 0.0 ):
            self.alfa1 = (c2.c.x-c1.c.x)/self.d
            self.alfa2 = (c2.c.y-c1.c.y)/self.d
            self.alfa3 = (c2.r*c2.r-c1.r*c1.r-self.d*self.d)/(2*self.d)
            self.alfa4 = (c2.cw*c2.r-c1.cw*c1.r)/self.d


        self.c1 = c1 # store all of c1 also??
    def getX(self):
        x = []
        x.append( self.c1.c.x )
        x.append( self.alfa1*self.alfa3 )
        x.append( self.alfa1*self.alfa4 )
        x.append( self.alfa2 )
        x.append( self.c1.r  )
        x.append( self.c1.cw )
        x.append( self.alfa3 )
        x.append( self.alfa4 )
        return x
    def getY(self):
        y = []
        y.append( self.c1.c.y )
        y.append( self.alfa2*self.alfa3 )
        y.append( self.alfa2*self.alfa4 )
        y.append( self.alfa1 )
        y.append( self.c1.r  )
        y.append( self.c1.cw )
        y.append( self.alfa3 )
        y.append( self.alfa4 )
        return y
        
class Bisector:
    def __init__(self, Bis):
        self.x= Bis.getX()
        self.y= Bis.getY()
    def Point(self, t, k):
        x=self.x
        y=self.y
        detx = ( math.pow((x[4]+x[5]*t),2) - math.pow((x[6]+x[7]*t),2) )
        dety = ( math.pow((y[4]+y[5]*t),2) - math.pow((y[6]+y[7]*t),2) )
        xp = x[0]-x[1]-x[2]*t + x[3]*math.sqrt( detx ) 
        yp = y[0]-y[1]-y[2]*t + y[3]*math.sqrt( dety )
        xm = x[0]-x[1]-x[2]*t - x[3]*math.sqrt( detx ) 
        ym = y[0]-y[1]-y[2]*t - y[3]*math.sqrt( dety ) 
        return [ocl.Point(xp,yp), ocl.Point(xm,ym)]
    def minT(self):
        # the minimum t that makes sense sets the sqrt() to zero
        # (x[4]+x[5]*t)^2 - (x[6]+x[7]*t)^2 = 0
        # (x[4]+x[5]*t)^2 = (x[6]+x[7]*t)^2
        # (x[4]+x[5]*t) = (x[6]+x[7]*t)  OR   (x[4]+x[5]*t) = -(x[6]+x[7]*t)
        # (x[5]-x[7])*t = (x[6]-x[4])  OR   (x[5]+x7*t) = x4-x[6]
        # t = x6-x4 / (x5-x7)    or    t = x4-x6 / (x5+x7)
        x = self.x 
        y = self.y 
        t1 = (x[6]-x[4]) / (x[5]-x[7])
        t2 = (-x[4]-x[6]) / (x[5]+x[7])
        t3 = (y[6]-y[4]) / (y[5]-y[7])
        t4 = (-y[4]-y[6]) / (y[5]+y[7])
        print " t1 solution= ",t1
        print " t2 solution= ",t2
        print " t3 solution= ",t3
        print " t4 solution= ",t4
        return t2
        
def drawBisector(myscreen, bis):
    N = 300
    t= bis.minT()
    tmax = 400
    dt = float(tmax)/float(N)
    ppts = []
    mpts = []
    for n in range(0,N):
        ppts.append( bis.Point(t,1)[0] )
        mpts.append( bis.Point(t,1)[1] )
        t= t+dt
    for p in ppts:
        drawVertex(myscreen, p, camvtk.green, rad=1)
    for p in mpts:
        drawVertex(myscreen, p, camvtk.red, rad=1)
        
        
# LINE/LINE

# LINE/CIRCLE



    
if __name__ == "__main__":  
    print ocl.revision()
    myscreen = camvtk.VTKScreen()
    myscreen.camera.SetPosition(0.01, 0,  1000 ) # 1200 for far view, 300 for circle view
    myscreen.camera.SetFocalPoint(0, 0, 0)
    myscreen.camera.SetClippingRange(-100,3000)
    camvtk.drawOCLtext(myscreen)
    w2if = vtk.vtkWindowToImageFilter()
    w2if.SetInput(myscreen.renWin)
    lwr = vtk.vtkPNGWriter()
    lwr.SetInput( w2if.GetOutput() )

    c1 = Circle(c=ocl.Point(100,30), r=100, cw=1)
    c2 = Circle(c=ocl.Point(20,30), r=60, cw=1)
    drawCircle(myscreen, c1, camvtk.cyan)
    drawCircle(myscreen, c2, camvtk.cyan)
    
    c1c2 = CircleCircle(c1,c2)
    bi = Bisector( c1c2 )
    drawBisector( myscreen,bi )
    myscreen.render()
    
    #w2if.Modified() 
    #lwr.SetFileName("frames/vd_dt_20_"+ ('%05d' % n)+".png")
    #lwr.Write()
    
    print "PYTHON All DONE."
    
    myscreen.render()    
    myscreen.iren.Start()
