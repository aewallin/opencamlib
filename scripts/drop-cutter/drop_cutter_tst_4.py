import ocl
import pyocl
import camvtk
import time
import vtk
import datetime
import math

if __name__ == "__main__":  
    print ocl.version()
    myscreen = camvtk.VTKScreen()
    
    a=ocl.Point(1,0,0)
    myscreen.addActor(camvtk.Point(center=(1,0,0), color=(1,0,1)));
    b=ocl.Point(0,1,0)    
    myscreen.addActor(camvtk.Point(center=(0,1,0), color=(1,0,1)));
    c=ocl.Point(0,0,0.3)
    myscreen.addActor( camvtk.Point(center=(0,0,0.3), color=(1,0,1)));
    myscreen.addActor( camvtk.Line(p1=(1,0,0),p2=(0,0,0.3)) )
    myscreen.addActor( camvtk.Line(p1=(0,0,0.3),p2=(0,1,0)) )
    myscreen.addActor( camvtk.Line(p1=(1,0,0),p2=(0,1,0)) )
    t = ocl.Triangle(a,b,c)
    s = ocl.STLSurf()
    s.addTriangle(t)
    
    diameter = 0.5234
    corneradius = 0.1
    angle = math.pi/4
    length = 5
    cutter = ocl.CylCutter(diameter, length)
    #cutter = ocl.BallCutter(diameter, length)
    #cutter = ocl.BullCutter(diameter,corneradius, length)
    #cutter = ocl.ConeCutter(diameter, angle, length)
    
    print cutter
    
    minx=-0.5
    dx=0.01
    maxx=1.5
    miny=-0.5
    dy=0.05
    maxy=1.5
    z=-0.8
    clpoints = pyocl.CLPointGrid(minx,dx,maxx,miny,dy,maxy,z)
    print len(clpoints), "cl-points to evaluate"
    n=0
    clp=[]
    bdc = ocl.BatchDropCutter()
    bdc.setSTL(s)
    bdc.setCutter(cutter)
    for p in clpoints:
        bdc.appendPoint(p)
        
    bdc.run()
   
    print "done."
    clpoints = bdc.getCLPoints()
    print "rendering..."
    print " len(clpoints)=", len(clpoints)

    camvtk.drawCLPointCloud(myscreen, clpoints)

    print "done."
        
    myscreen.camera.SetPosition(0.5, 3, 2)
    myscreen.camera.SetFocalPoint(0.5, 0.5, 0)
    myscreen.render()
    
    #w2if = vtk.vtkWindowToImageFilter()
    #w2if.SetInput(myscreen.renWin)
    #lwr = vtk.vtkPNGWriter()
    #lwr.SetInput( w2if.GetOutput() )
    
    t = camvtk.Text()
    t.SetPos( (myscreen.width-350, myscreen.height-30) )
    myscreen.addActor(t)
        
    #for n in range(1,18):
    #    t.SetText("OpenCAMLib " + datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S"))
    #    myscreen.camera.Azimuth( 2 )
    #    time.sleep(0.1)
    #    myscreen.render()
    #    w2if.Modified()
    #    lwr.SetFileName("frames/tc"+ ('%04d' % n)+".png")
        #lwr.Write()



    myscreen.iren.Start()
    #raw_input("Press Enter to terminate") 
    
