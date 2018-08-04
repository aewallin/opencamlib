import ocl
import camvtk
import time
import vtk
import datetime
import math

def drawPoints(myscreen, clpoints, ccpoints):
    c=camvtk.PointCloud( pointlist=clpoints, collist=ccpoints) 
    c.SetPoints()
    myscreen.addActor(c )

def drawFiber(myscreen, f, fibercolor=camvtk.red):
    inter = f.getInts()
    for i in inter:
        if not i.empty():
            ip1 = f.point( i.lower )
            ip2 = f.point( i.upper )
            myscreen.addActor( camvtk.Line(p1=(ip1.x,ip1.y,ip1.z),p2=(ip2.x,ip2.y,ip2.z), color=fibercolor) )
            myscreen.addActor( camvtk.Sphere(center=(ip1.x,ip1.y,ip1.z),radius=0.005, color=camvtk.clColor( i.lower_cc) ) )
            myscreen.addActor( camvtk.Sphere(center=(ip2.x,ip2.y,ip2.z),radius=0.005, color=camvtk.clColor( i.upper_cc) ) )
            cc1 = i.lower_cc
            cc2 = i.upper_cc
            myscreen.addActor( camvtk.Sphere(center=(cc1.x,cc1.y,cc1.z),radius=0.005, color=camvtk.lgreen ) )
            myscreen.addActor( camvtk.Sphere(center=(cc2.x,cc2.y,cc2.z),radius=0.005, color=camvtk.lgreen ) )
            # cutter circle
            #c1 = camvtk.Circle(center=(ip1.x,ip1.y,ip1.z), radius = 0.3/2, color=fibercolor)
            #myscreen.addActor(c1)
            #c2 = camvtk.Circle(center=(ip2.x,ip2.y,ip2.z), radius = 0.3/2, color=fibercolor)
            #myscreen.addActor(c2)

def drawFiber_clpts(myscreen, f, fibercolor=camvtk.red):
    inter = f.getInts()
    for i in inter:
        if not i.empty():
            ip1 = f.point( i.lower )
            ip2 = f.point( i.upper )
            myscreen.addActor( camvtk.Line(p1=(ip1.x,ip1.y,ip1.z),p2=(ip2.x,ip2.y,ip2.z), color=fibercolor) )
            myscreen.addActor( camvtk.Sphere(center=(ip1.x,ip1.y,ip1.z),radius=0.005, color=camvtk.clColor( i.lower_cc) ) )
            myscreen.addActor( camvtk.Sphere(center=(ip2.x,ip2.y,ip2.z),radius=0.005, color=camvtk.clColor( i.upper_cc) ) )
            #cc1 = i.lower_cc
            #cc2 = i.upper_cc
            #myscreen.addActor( camvtk.Sphere(center=(cc1.x,cc1.y,cc1.z),radius=0.005, color=camvtk.lgreen ) )
            #myscreen.addActor( camvtk.Sphere(center=(cc2.x,cc2.y,cc2.z),radius=0.005, color=camvtk.lgreen ) )

def yfiber(yvals,s,zh,myscreen):
    for y in yvals:
        f1 = ocl.Point(-20,y,zh) # start point of fiber
        f2 = ocl.Point(+20,y,zh)  # end point of fiber
        f =  ocl.Fiber( f1, f2)
        for t in s.getTriangles():
            i = ocl.Interval()
            #cutter.vertexPush(f,i,t)
            #cutter.facetPush(f,i,t)
            #cutter.edgePush(f,i,t)
            cutter.pushCutter(f,i,t)
            f.addInterval(i)
        drawFiber_clpts(myscreen, f, camvtk.red)

def xfiber(xvals,s,zh,myscreen):
    for x in xvals:
        f1 = ocl.Point(x,-20,zh) # start point of fiber
        f2 = ocl.Point(x,+20,zh)  # end point of fiber
        f =  ocl.Fiber( f1, f2)
        for t in s.getTriangles():
            i = ocl.Interval()
            #cutter.vertexPush(f,i,t)
            #cutter.facetPush(f,i,t)
            #cutter.edgePush(f,i,t)
            cutter.pushCutter(f,i,t)
            f.addInterval(i)
        drawFiber_clpts(myscreen, f, camvtk.lblue)
        
if __name__ == "__main__":  
    print ocl.version() 
    myscreen = camvtk.VTKScreen()
    #stl = camvtk.STLSurf("../stl/gnu_tux_mod.stl")
    stl = camvtk.STLSurf("../../stl/demo.stl")
    myscreen.addActor(stl)
    stl.SetWireframe()
    stl.SetColor((1,1,1))
    polydata = stl.src.GetOutput()
    s = ocl.STLSurf()
    camvtk.vtkPolyData2OCLSTL(polydata, s)
    print "STL surface read,", s.size(), "triangles"
    
    cutter = ocl.CylCutter(0.3, 6)
    print "lengt=", cutter.getLength()
    print "fiber...",
    range=30
    Nmax = 200
    yvals = [float(n-float(Nmax)/2)/Nmax*range for n in xrange(0,Nmax+1)]
    xvals = [float(n-float(Nmax)/2)/Nmax*range for n in xrange(0,Nmax+1)]
    zmin = -0.1
    zmax = 0.5
    zNmax = 2
    dz = (zmax-zmin)/(zNmax-1)
    zvals=[]
    #for n in xrange(0,zNmax):
    #    zvals.append(zmin+n*dz)
    zvals.append(0.1)
    #zvals = [ float(n-float(zNmax)/2)/zNmax*range for n in xrange(0,zNmax+1)]
    #print zvals
    #exit()
    #cc = ocl.CCPoint()
    #zh = -0.1
    #zh = 0.2571567
    
    for zh in zvals:
        print "fibers at z=",zh
        yfiber(yvals,s,zh,myscreen)
        xfiber(xvals,s,zh,myscreen)
    
    print "done."
    myscreen.camera.SetPosition(0.5, 3, 2)
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
