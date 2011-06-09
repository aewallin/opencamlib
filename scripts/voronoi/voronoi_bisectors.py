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

def drawLine(myscreen, l, lineColor):
    # a x + b y + c = 0
    # x = -c/a
    p1 = 100*ocl.Point( -l.c/l.a , 0 )
    p2 = 100*ocl.Point( 0, -l.c/l.b )
    myscreen.addActor( camvtk.Line( p1=( p1.x,p1.y,p1.z), p2=(p2.x,p2.y,p2.z), color=lineColor ) )
    
# CIRCLE def
# (x(t) - xc1)^2 + (y(t)-yc1)^2 = (r1+k1*t)^2 
class Circle:
    def __init__(self,c=ocl.Point(0,0),r=1,cw=1):
        self.c = c
        self.r = r
        self.cw = cw # CW=1, CCW = -1
        # k +1 enlarging circle
        # k -1 shrinking circle 

# LINE def
# a1 x + b1 y + c1 + k1 t = 0 and a*a + b*b = 1 
class Line:
    def __init__(self,a,b,c,k):
        self.a = a
        self.b = b
        self.c = c
        self.k = k # offset to left or right of line

# from Held 1991, page 94->
#
# bisectors are of the form
# line, parabola, ellipse, hyperbola
#     x(t) = x1 - x2 - x3*t +/- x4 sqrt( square(x5+x6*t) - square(x7+x8*t) )
#     y(t) = y1 - y2 - y3*t +/- y4 sqrt( square(y5+y6*t) - square(y7+y8*t) ) 
# line/line: line
# circle/line: parabola
# circle/circle: ellipse/hyperbola
# !only valid if no parallel lines and no concentric arcs
#
# line:   (a, b, c, k)     
#         a1 x + b1 y + c1 + k1 t = 0 and a*a + b*b = 1
#         k= +/- 1 indicates offset to right/left
#
# circle: (xc, yc, r, lambda) 
#         (x(t) - xc1)^2 + (y(t)-yc1)^2 = (r1+k1*t)^2
#        lambda=-1 for CCW arc and +1 otherwise 
#         k +1 enlarging circle, k -1 shrinking circle
#
# for a bisector we store only four parameters (alfa1, alfa2, alfa3, alfa4)
#
# line/line
#    delta = a1*b2 - a2*b1
#    alfa1= (b1*d2-b2*d1)/delta
#    alfa2= (a2*d1-a1*d2)/delta
#    alfa3= b2-b1
#    alfa4= a1-a2 
#   bisector-params:
#    x1 = alfa1, x3 = -alfa3, x2 = x4 = x5 = x6 = x7 = x8 = 0
#    y1 = alfa2, y3 = -alfa4, y2=y4=y5=y6=y7=y8 = 0 
#
# circle/line
#     
#    alfa1= a2
#    alfa2= b2
#    alfa3= a2*xc1 + b2*yc1+d2
#    alfa4= r1
#   params: 
#    x1 = xc1, x2 = alfa1*alfa3, x3 = -alfa1, x3 = alfa2, x5 = alfa4, x6 = lambda1, x7 = alfa3, x8 = -1
#    y1 = yc1, y2 = alfa2*alfa3, y3 = -alfa2, y4 = alfa1, y5 = alfa4, y6 = lambda1, y7 = alfa3, y8 = -1 
#
# circle / circle
#    d= sqrt( square(xc1-xc2) + square(yc1-yc2) ) 
#    alfa1= (xc2-xc1)/d
#    alfa2= (yc2-yc1)/d
#    alfa3= (r2*r2-r1*r1-d*d)/2d
#    alfa4= (lambda2*r2-lambda1*r1)/d 
#   params:
#    x1 = xc1, x2 = alfa1*alfa3, x3 = alfa1*alfa4, x4 = alfa2, x5 = r1, x6 = lambda1, x7 = alfa3, x8 = alfa4
#    y1 = yc1, y2 = alfa2*alfa3, y3 = alfa2*alfa4, y4 = alfa1, y5 = r1, y6 = lambda1, y7 = alfa3, y8 = alfa4 

class LineLine:
    """ line/line bisector is a line """
    def __init__(self,l1,l2):
        self.delta= l1.a*l2.b-l2.a*l1.b
        self.alfa1 = (l1.b*l2.c-l2.b*l1.c) / self.delta
        self.alfa2 = (l2.a*l1.c-l1.a*l2.c) / self.delta
        self.alfa3 = l2.b*l1.k-l1.b*l2.k
        self.alfa4 = l1.a*l2.k-l2.a*l1.k
    def getX(self):
        x = []
        x.append( self.alfa1 )
        x.append( 0 )
        x.append( -self.alfa3 )
        x.append( 0 )
        x.append( 0 )
        x.append( 0 )
        x.append( 0 )
        x.append( 0 )
        return x
    def getY(self):
        y = []
        y.append( self.alfa2 )
        y.append( 0 )
        y.append( -self.alfa4 )
        y.append( 0 )
        y.append( 0 )
        y.append( 0 )
        y.append( 0 )
        y.append( 0 )
        return y

# CIRCLE/LINE (same as point-line?)
#    * alfa1= a2
#    * alfa2= b2
#    * alfa3= a2*xc1 + b2*yc1+d2 (c2?)
#    * alfa4= r1 
#       x1 = xc1
#       x2 = alfa1*alfa3
#       x3 = -alfa1, 
#       x3 = alfa2, 
#       x5 = alfa4, 
#       x6 = lambda1, 
#       x7 = alfa3, 
#       x8 = -1
#       y1 = yc1, 
#       y2 = alfa2*alfa3, 
#       y3 = -alfa2, 
#       y4 = alfa1, 
#       y5 = alfa4, 
#       y6 = lambda1, 
#       y7 = alfa3, 
#       y8 = -1 


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
        t1=0
        t2=0
        t3=0
        t4=0
        if (((x[5]-x[7])!=0) and ((x[5]+x[7])!=0) ):
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

if __name__ == "__main__":  
    print ocl.revision()
    myscreen = camvtk.VTKScreen()
    myscreen.camera.SetPosition(0.01, 0,  1000 ) 
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
    bicc = Bisector( c1c2 )
    drawBisector( myscreen, bicc )
    
    c1a = Circle(c=ocl.Point(100,30), r=100, cw=-1)
    c2a  = Circle(c=ocl.Point(20,30), r=60, cw=1)
    c1c2alt = CircleCircle(c1a,c2a)
    biccalt = Bisector( c1c2alt )
    drawBisector( myscreen, biccalt )

    
    
    l1 = Line( math.cos(1),   math.sin(1)   , 1 , -1)
    l2 = Line( math.cos(0.1), math.sin(0.1) , -1, 1)
    drawLine(myscreen, l1, camvtk.yellow )
    drawLine(myscreen, l2, camvtk.yellow )
    
    l1l2 = LineLine( l1, l2)
    bill = Bisector( l1l2 )
    drawBisector( myscreen, bill )
    
    myscreen.render()
    
    #w2if.Modified() 
    #lwr.SetFileName("frames/vd_dt_20_"+ ('%05d' % n)+".png")
    #lwr.Write()
    
    print "PYTHON All DONE."
    
    myscreen.render()    
    myscreen.iren.Start()
