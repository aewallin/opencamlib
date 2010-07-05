import ocl
import pyocl
import camvtk
import time
import vtk
import datetime

if __name__ == "__main__":  
    print ocl.revision()
    myscreen = camvtk.VTKScreen()
    myscreen.setAmbient(20,20,20)
    
    myscreen.camera.SetPosition(4, 4, 3)
    myscreen.camera.SetFocalPoint(0.6, 0.6, 0)
    myscreen.setAmbient(1,1,1)

    a = ocl.Point(1,0,0)
    b = ocl.Point(0,1,0)    
    c = ocl.Point(0,0,0.3)
    
    myscreen.addActor( camvtk.Point(center=(a.x,a.y,a.z), color=(1,0,1)));
    myscreen.addActor( camvtk.Point(center=(b.x,b.y,b.z), color=(1,0,1)));
    myscreen.addActor( camvtk.Point(center=(c.x,c.y,c.z), color=(1,0,1)));
    myscreen.addActor( camvtk.Line(p1=(a.x,a.y,a.z),p2=(c.x,c.y,c.z)) )
    myscreen.addActor( camvtk.Line(p1=(c.x,c.y,c.z),p2=(b.x,b.y,b.z)) )
    myscreen.addActor( camvtk.Line(p1=(a.x,a.y,a.z),p2=(b.x,b.y,b.z)) )
    t = ocl.Triangle(a,b,c)
    
    #cutter = ocl.BullCutter(1,0.2)
    #cutter = ocl.CylCutter(0.5)
    cutter = ocl.BallCutter(0.5)
    
    
    print cutter
    
    
    # grid parameters
    minx=-0.7
    dx=0.03
    maxx=1.7
    miny=-0.7
    dy=0.03
    maxy=1.7
    z=-0.5
    # generate list of CL-poins at height z
    clpoints = pyocl.CLPointGrid(minx,dx,maxx,miny,dy,maxy,z)

    print len(clpoints), "cl-points to evaluate"

    
    tx = camvtk.Text()
    tx.SetPos( (myscreen.width-200, myscreen.height-30) )
    myscreen.addActor( tx )
    
    w2if = vtk.vtkWindowToImageFilter()
    w2if.SetInput(myscreen.renWin)
    lwr = vtk.vtkPNGWriter()
    lwr.SetInput( w2if.GetOutput() )
    w2if.Modified()
    
    # loop through the cl-points
    n=0
    for cl in clpoints:

        cutter.vertexDrop(cl,t)
        cutter.edgeDrop(cl,t)
        cutter.facetDrop(cl,t)

        n=n+1
        if (n % int(len(clpoints)/10)) == 0:
            print n/int(len(clpoints)/10), " ",
            
    print "done."
    print "rendering...",
    # render all the points
    camvtk.drawCLPoints(myscreen, clpoints)
    camvtk.drawCCPoints(myscreen, clpoints)
    print "done."   
    
    # animate by rotating the camera
    for n in range(1,90):
        tx.SetText(datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S"))
        myscreen.camera.Azimuth( 4 )
        #time.sleep(0.01)
        myscreen.render()
        lwr.SetFileName("frames/ball_all"+ ('%05d' % n)+".png")
        w2if.Modified() 
        #lwr.Write() # write screenshot to file

    myscreen.iren.Start()
    raw_input("Press Enter to terminate") 
    
