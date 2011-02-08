import ocl
import camvtk
import time
import vtk
import datetime
import math
import random
import gc

def drawVertex(myscreen, p, vertexColor, rad=1):
    myscreen.addActor( camvtk.Sphere( center=(p.x,p.y,p.z), radius=rad, color=vertexColor ) )

def drawEdge(myscreen, e, edgeColor=camvtk.yellow):
    p1 = e[0]
    p2 = e[1]
    myscreen.addActor( camvtk.Line( p1=( p1.x,p1.y,p1.z), p2=(p2.x,p2.y,p2.z), color=edgeColor ) )

def drawCircle(myscreen, p, r, circleColor):
    myscreen.addActor( camvtk.Circle( center=(p.x,p.y,p.z), radius=r, color=circleColor ) )

class Circle:
    def __init__(self,c=ocl.Point(0,0),r=1,k=1):
        self.c = c
        self.r = r
        self.k = 1

class Line:
    def __init__(self,a,b,c):
        self.a=a
        self.b=b
        selc.c=c

class Bisector:
    def __init__(self, myscreen):
        self.alfa=[]
        self.gen1
        self.gen2
        self.delta = a1*b2-a2*b1
    def Point(self, t, k):
        self.CalcParams()
        x = x1 - x2 - x3*t +/- x4 * math.sqrt( (x5+x6*t) - square(x7+x8*t) ) 
        y = y1 - y2 - y3*t +/- y4 math.sqrt( (y5+y6*t) - square(y7+y8*t) ) 
        return ocl.Point(x,y)

# LINE def
# a1 x + b1 y + c1 + k1 t = 0 and a*a + b*b = 1 

# CIRCLE def
# (x(t) - xc1)^2 + (y(t)-yc1)^2 = (r1+k1*t)^2 

# LINE/LINE

# LINE/CIRCLE

# CIRCLE / CIRCLE
# d= sqrt( square(xc1-xc2) + square(yc1-yc2) ) 
# lambda=-1 for CCW arc and +1 otherwise 

    
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
    #w2if.Modified()
    #lwr.SetFileName("tux1.png")
    
    
    myscreen.render()
    #random.seed(42)
    c1 = ocl.Point(-100,30)
    r1 = 100
    
    c2 = ocl.Point(50,-25)
    r2 = 31.4
    #vod.setAll(vd)
    drawCircle(myscreen, c1, r1, camvtk.cyan)
    drawCircle(myscreen, c2, r2, camvtk.cyan)
    #drawLine(myscreen, camvtk.cyan)
    
    
    
    

    
    
    #w2if.Modified() 
    #lwr.SetFileName("frames/vd_dt_20_"+ ('%05d' % n)+".png")
    #lwr.Write()
    
    print "PYTHON All DONE."
    
    myscreen.render()    
    myscreen.iren.Start()
