import ocl
import pyocl
import camvtk
import vtk
import math

if __name__ == "__main__":  
    print(ocl.version()) # print out git version tag
    
    # set up VTK visualization
    myscreen = camvtk.VTKScreen()
    myscreen.setAmbient(20,20,20)
    myscreen.camera.SetPosition(4, 4, 3)
    myscreen.camera.SetFocalPoint(0.6, 0.6, 0)
    myscreen.setAmbient(1,1,1)
    
    #camvtk.drawArrows(myscreen)
    
    # three corners of a triangle
    a = ocl.Point(1,0,-0.000010)
    b = ocl.Point(0,1,+0.0)    
    c = ocl.Point(0.001,0,+0.3001)
    #c = ocl.Point(0,0,0.3)
    t = ocl.Triangle(a,b,c)
    
    # draw the triangle with VTK
    myscreen.addActor( camvtk.Point(center=(a.x,a.y,a.z), color=(1,0,1)));
    myscreen.addActor( camvtk.Point(center=(b.x,b.y,b.z), color=(1,0,1)));
    myscreen.addActor( camvtk.Point(center=(c.x,c.y,c.z), color=(1,0,1)));
    myscreen.addActor( camvtk.Line(p1=(a.x,a.y,a.z),p2=(c.x,c.y,c.z)) )
    myscreen.addActor( camvtk.Line(p1=(c.x,c.y,c.z),p2=(b.x,b.y,b.z)) )
    myscreen.addActor( camvtk.Line(p1=(a.x,a.y,a.z),p2=(b.x,b.y,b.z)) )

    # Define a cutter 
    angle = math.pi/8
    length = 5.0
    #c1 = ocl.BullCutter(0.5,0.1, length)
    #c1 = ocl.CylCutter(0.5, length)
    #c1 = ocl.BallCutter(0.5, length)
    c1 = ocl.ConeCutter(0.5,angle, length)    
    cutter = c1
    cutter = c1.offsetCutter(0.1)
    
    print(cutter)
    
    
    # grid parameters
    minx=-0.7
    dx=0.03
    maxx=1.7
    miny=-0.7
    dy=0.03
    maxy=1.7
    z=-0.5
    # generate list of CL-poins at height z
    clpoints=[]
    # we calculate cutter-locations for this grid of XY points
    clpoints = pyocl.CLPointGrid(minx,dx,maxx,miny,dy,maxy,z)
    
    # loop through the cl-points
    n=0
    print(len(clpoints), "cl-points to evaluate")
    for cl in clpoints:
        cutter.vertexDrop(cl,t)
        cutter.edgeDrop(cl,t)
        cutter.facetDrop(cl,t)
        n=n+1
    print("drop-cutter done.")
    # cl has now been updated so it is a valid cutter-location point
    # drop-cutter only updates the z-coordinate of cl, x/y are not changed.

    print("rendering...",)
    # render all the points
    camvtk.drawCLPointCloud(myscreen, clpoints)
    camvtk.drawCCPoints(myscreen, clpoints)
    print("done."   )
    
    tx = camvtk.Text()
    tx.SetPos( (myscreen.width-200, myscreen.height-130) )
    tx.SetText("opencamlib\ndrop-cutter")
    myscreen.addActor( tx )
    
    #w2if = vtk.vtkWindowToImageFilter()
    #w2if.SetInput(myscreen.renWin)
    #lwr = vtk.vtkPNGWriter()
    #lwr.SetInput( w2if.GetOutput() )
    #w2if.Modified()
    
    
    # animate by rotating the camera
    """
    for n in range(1,90):
        tx.SetText(datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S"))
        myscreen.camera.Azimuth( 4 )
        #time.sleep(0.01)
        myscreen.render()
        lwr.SetFileName("frames/ball_all"+ ('%05d' % n)+".png")
        w2if.Modified() 
        #lwr.Write() # write screenshot to file
    """
    myscreen.render()
    myscreen.iren.Start()
    raw_input("Press Enter to terminate") 
    
