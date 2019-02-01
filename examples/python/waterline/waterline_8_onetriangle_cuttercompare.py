import ocl
import camvtk
import time
import vtk
import datetime
import math

def calcWaterline(zh, cutter,s):
    #wl = ocl.Waterline()
    wl = ocl.AdaptiveWaterline()
    wl.setSTL(s)
    wl.setCutter(cutter)
    wl.setZ(zh)
    wl.setSampling(0.01)
    wl.setThreads(4)
    t_before = time.time() 
    wl.run()
    t_after = time.time()
    calctime = t_after-t_before
    print(" Waterline done in ", calctime," s")
    out = []
    out.append(wl.getLoops())
    out.append(wl.getXFibers())
    out.append(wl.getYFibers())
    return out
    
def drawFiber(myscreen, f, fibercolor=camvtk.red):
    inter = f.getInts()
    for i in inter:
        if not i.empty():
            ip1 = f.point( i.lower )
            ip2 = f.point( i.upper )
            ballradius = 0.0005
            myscreen.addActor( camvtk.Line(p1=(ip1.x,ip1.y,ip1.z),p2=(ip2.x,ip2.y,ip2.z), color=fibercolor) )
            myscreen.addActor( camvtk.Sphere(center=(ip1.x,ip1.y,ip1.z),radius=ballradius, color=camvtk.clColor( i.lower_cc) ) )
            myscreen.addActor( camvtk.Sphere(center=(ip2.x,ip2.y,ip2.z),radius=ballradius, color=camvtk.clColor( i.upper_cc) ) )
            cc1 = i.lower_cc
            cc2 = i.upper_cc
            myscreen.addActor( camvtk.Sphere(center=(cc1.x,cc1.y,cc1.z),radius=ballradius, color=camvtk.lgreen ) )
            myscreen.addActor( camvtk.Sphere(center=(cc2.x,cc2.y,cc2.z),radius=ballradius, color=camvtk.lgreen ) )
            # cutter circle
            #c1 = camvtk.Circle(center=(ip1.x,ip1.y,ip1.z), radius = 0.3/2, color=fibercolor)
            #myscreen.addActor(c1)
            #c2 = camvtk.Circle(center=(ip2.x,ip2.y,ip2.z), radius = 0.3/2, color=fibercolor)
            #myscreen.addActor(c2)

def drawLoops(myscreen, loops, loopColor=camvtk.yellow):
    nloop=0
    zofz = 0.00
    for lop in loops:
        n = 0
        N = len(lop)
        first_point=ocl.Point(-1,-1,5)
        previous=ocl.Point(-1,-1,5)
        for p in lop:
            if n==0: # don't draw anything on the first iteration
                previous=p 
                first_point = p
            elif n== (N-1): # the last point
                myscreen.addActor( camvtk.Line(p1=(previous.x,previous.y,previous.z+zofz),p2=(p.x,p.y,p.z+zofz),color=loopColor) ) # the normal line
                # and a line from p to the first point
                myscreen.addActor( camvtk.Line(p1=(p.x,p.y,p.z+zofz),p2=(first_point.x,first_point.y,first_point.z+zofz),color=loopColor) )
            else:
                myscreen.addActor( camvtk.Line(p1=(previous.x,previous.y,previous.z+zofz),p2=(p.x,p.y,p.z+zofz),color=loopColor) )
                previous=p
            n=n+1
        zofz = zofz +0.00
        print("rendered loop ",nloop, " with ", len(lop), " points at zofz=",zofz)
        if len(lop)==2:
            for p in lop:
                print(p)
                myscreen.addActor( camvtk.Sphere(center=(p.x,p.y,p.z+zofz),radius=0.0005, color=camvtk.pink ) )
        nloop = nloop+1

if __name__ == "__main__":  
    print(ocl.version())
    myscreen = camvtk.VTKScreen()
    a = ocl.Point(0,1,0.3)
    myscreen.addActor(camvtk.Point(center=(a.x,a.y,a.z), color=(1,0,1)))
    b = ocl.Point(1,0.5,0.3)    
    myscreen.addActor(camvtk.Point(center=(b.x,b.y,b.z), color=(1,0,1)))
    c = ocl.Point(-0.1,0.3,0.0)
    myscreen.addActor(camvtk.Point(center=(c.x,c.y,c.z), color=(1,0,1)))
    myscreen.addActor( camvtk.Line(p1=(a.x,a.y,a.z),p2=(c.x,c.y,c.z)) )
    myscreen.addActor( camvtk.Line(p1=(c.x,c.y,c.z),p2=(b.x,b.y,b.z)) )
    myscreen.addActor( camvtk.Line(p1=(a.x,a.y,a.z),p2=(b.x,b.y,b.z)) )
    t = ocl.Triangle(b,c,a)
    s = ocl.STLSurf()
    s.addTriangle(t) # a one-triangle STLSurf
    zheights=[-0.3, -0.2, -0.1, -0.05, 0.0, 0.05, 0.1, 0.15, 0.2, 0.25, 0.26, 0.27, 0.28, 0.29 ]  # the z-coordinates for the waterlines
    zheights=[-0.8, -0.7, -0.6, -0.5, -0.4, -0.3, -0.2, -0.1, -0.05, 0.0, 0.05, 0.1, 0.15, 0.2,  0.28 ]
    zheights=[ -0.35, -0.3, -0.25, -0.2, -0.15, -0.1, -0.05, 0.0, 0.05, 0.1, 0.15, 0.2,  0.25, 0.28]
    
    zheights=[]
    Nmax=10
    zmin=-0.5
    zmax= 0.30
    dz = (zmax-zmin)/float(Nmax-1)
    z = zmin
    for n in range(Nmax):
        zheights.append(z)
        z=z+dz
        
    #zheights=[]
    """
    zheights.append(0.29)
    zheights.append(0.28)
    zheights.append(0.27)
    zheights.append(0.26)
    zheights.append(0.25)
    """
    #zheights=[ -0.35,  -0.25,  -0.15,  -0.05, 0.05,  0.15,   0.25]
    #zheights=[ 0.1]
    
    length = 10
    diam = 0.6
    cutter1 = ocl.CylCutter( diam , length )
    cutter2 = ocl.BallCutter( diam , length )
    cutter3 = ocl.BullCutter( diam , diam/5, length )
    cutter4 = ocl.ConeCutter( diam , math.pi/5, length )
    cutter5 =  ocl.CylConeCutter(diam/float(3),diam,math.pi/float(9))
    
    for zh in zheights:
        loops = calcWaterline(zh, cutter5, s)
        drawLoops(myscreen, loops[0], camvtk.red)
        
        #loops = calcWaterline(zh, cutter2, s)
        #drawLoops(myscreen, loops[0], camvtk.green)
        #loops = calcWaterline(zh, cutter3, s)
        #drawLoops(myscreen, loops[0], camvtk.yellow)
        
        #loops = calcWaterline(zh, cutter4, s)
        #drawLoops(myscreen, loops[0], camvtk.pink)
        
        #for f in loops[1]:
        #    drawFiber(myscreen, f, camvtk.red)
        #for f in loops[2]:
        #    drawFiber(myscreen, f, camvtk.lblue)
        
        
        

    print("done.")
    myscreen.camera.SetPosition(1, -1, 3)
    myscreen.camera.SetFocalPoint(0.5, 0.5, 0)
    camvtk.drawArrows(myscreen,center=(-0.5,-0.5,-0.5))
    camvtk.drawOCLtext(myscreen)
    myscreen.render()    
    myscreen.iren.Start()
    #raw_input("Press Enter to terminate") 
