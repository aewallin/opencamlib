import ocl
import camvtk
import time
import vtk
import datetime
import math

# 2018.08: Weave not wrapped

def loop_waterline(zh, cutter,s):
    fiber_range=2
    Nmax = 200
    yvals = [float(n-float(Nmax)/2)/Nmax*fiber_range for n in range(0,Nmax+1)]
    xvals = [float(n-float(Nmax)/2)/Nmax*fiber_range for n in range(0,Nmax+1)]
    bpc = ocl.BatchPushCutter()
    bpc.setXDirection()
    bpc.setSTL(s)
    bpc.setCutter(cutter)
    for y in yvals:
        f1 = ocl.Point(-fiber_range,y,zh) # start point of fiber
        f2 = ocl.Point(+fiber_range,y,zh)  # end point of fiber
        f =  ocl.Fiber( f1, f2)
        bpc.appendFiber(f)
    for x in xvals:
        f1 = ocl.Point(x,-fiber_range,zh) # start point of fiber
        f2 = ocl.Point(x,+fiber_range,zh)  # end point of fiber
        f =  ocl.Fiber( f1, f2)
        bpc.appendFiber(f)
    bpc.run()
    clpoints = bpc.getCLPoints()
    fibers = bpc.getFibers()
    w = ocl.Weave()
    print("Weave...",)
    for f in fibers:
        w.addFiber(f)
    print("build()...",)
    w.build()
    print("face_traverse()...",)
    w.face_traverse()
    print("done.")
    return w.getLoops()

def drawLoop(myscreen, w_loop):  # draw the loop as a yellow line
    previous = 0
    for loop in w_loop:
        np = 0
        for p in loop:      
            if np is not 0:
                myscreen.addActor( camvtk.Line( p1=(previous.x,previous.y, previous.z), 
                                                p2=(p.x,p.y,p.z), color=camvtk.yellow) )
            np=np+1
            previous = p

if __name__ == "__main__":  
    print(ocl.version())
    myscreen = camvtk.VTKScreen()
    a = ocl.Point(0,1,0.3)
    myscreen.addActor(camvtk.Point(center=(a.x,a.y,a.z), color=(1,0,1)))
    b = ocl.Point(1,0.5,0.3)    
    myscreen.addActor(camvtk.Point(center=(b.x,b.y,b.z), color=(1,0,1)))
    c = ocl.Point(0,0,0.1)
    myscreen.addActor(camvtk.Point(center=(c.x,c.y,c.z), color=(1,0,1)))
    myscreen.addActor( camvtk.Line(p1=(a.x,a.y,a.z),p2=(c.x,c.y,c.z)) )
    myscreen.addActor( camvtk.Line(p1=(c.x,c.y,c.z),p2=(b.x,b.y,b.z)) )
    myscreen.addActor( camvtk.Line(p1=(a.x,a.y,a.z),p2=(b.x,b.y,b.z)) )
    t = ocl.Triangle(b,c,a)
    s = ocl.STLSurf()
    s.addTriangle(t) # a one-triangle STLSurf
    diameter = 0.4
    angle = math.pi/4
    length = 5
    cutter = ocl.CylCutter(0.3, 5)
    #cutter = ocl.BallCutter(0.4, 5)
    #cutter = ocl.BullCutter(0.4, 0.1, 5)
    #cutter = ocl.ConeCutter(diameter, angle, length)
    print(cutter)
    zstart = 0
    zend = 0.35
    zvals=[]
    Nz = 7
    for n in range(0,Nz):
        zvals.append( zstart + n*(zend-zstart)/float(Nz-1) )
    for zh in zvals:
        print("zh=", zh)
        w_loop = loop_waterline(zh, cutter,s)
        drawLoop( myscreen, w_loop )
        
    print("done.")
    myscreen.camera.SetPosition(0.5, 3, 2)
    myscreen.camera.SetFocalPoint(0.5, 0.5, 0)
    camvtk.drawArrows(myscreen,center=(-0.5,-0.5,-0.5))
    camvtk.drawOCLtext(myscreen)

    myscreen.render()    

    myscreen.iren.Start()
    #raw_input("Press Enter to terminate") 
