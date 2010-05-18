import ocl
import camvtk
import time
import vtk
import datetime


def CLPointGrid(minx,dx,maxx,miny,dy,maxy,z):
    """ generate and return a rectangular grid of points """
    plist = []
    xvalues = [round(minx+n*dx,2) for n in xrange(int(round((maxx-minx)/dx))+1) ]
    yvalues = [round(miny+n*dy,2) for n in xrange(int(round((maxy-miny)/dy))+1) ]
    for y in yvalues:
        for x in xvalues:
            plist.append( ocl.Point(x,y,z) )
    return plist

if __name__ == "__main__":  
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
    
    print ocl.revision()
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
    clpoints = CLPointGrid(minx,dx,maxx,miny,dy,maxy,z)

    print len(clpoints), "cl-points to evaluate"
    n=0
    ccpoints=[]
    
    tx = camvtk.Text()
    tx.SetPos( (myscreen.width-200, myscreen.height-30) )
    myscreen.addActor( tx )
    
    w2if = vtk.vtkWindowToImageFilter()
    w2if.SetInput(myscreen.renWin)
    lwr = vtk.vtkPNGWriter()
    lwr.SetInput( w2if.GetOutput() )
    w2if.Modified()
    
    # loop through the cl-points
    for cl in clpoints:
        cc = ocl.CCPoint()
        cutter.vertexDrop(cl,cc,t)
        cutter.edgeDrop(cl,cc,t)
        cutter.facetDrop(cl,cc,t)
        ccpoints.append(cc)
        n=n+1
        if (n % int(len(clpoints)/10)) == 0:
            print n/int(len(clpoints)/10), " ",
            
    print "done."
    print "rendering...",
    # render all the points
    for cl,cc in zip(clpoints,ccpoints):
        myscreen.addActor( camvtk.Point(center=(cl.x,cl.y,cl.z) , color=camvtk.ccColor(cc) ) )
        
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
    
