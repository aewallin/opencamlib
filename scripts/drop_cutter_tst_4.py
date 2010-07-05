import ocl
import camvtk
import time
import vtk
import datetime
import math

def CLPointGrid(minx,dx,maxx,miny,dy,maxy,z):
    plist = []
    xvalues = [round(minx+n*dx,2) for n in xrange(int(round((maxx-minx)/dx))+1) ]
    yvalues = [round(miny+n*dy,2) for n in xrange(int(round((maxy-miny)/dy))+1) ]
    for y in yvalues:
        for x in xvalues:
            plist.append( ocl.CLPoint(x,y,z) )
    return plist



if __name__ == "__main__":  
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
    
    radius1=1
    
    cutter = ocl.CylCutter(1.234)
    
    angle = math.pi/4
    #cutter = ocl.ConeCutter(1, angle)
    
    print cutter
    
    minx=-0.5
    dx=0.02
    maxx=1.5
    miny=-0.5
    dy=0.02
    maxy=1.5
    z=-0.8
    clpoints = CLPointGrid(minx,dx,maxx,miny,dy,maxy,z)
    print len(clpoints), "cl-points to evaluate"
    n=0
    
    for cl in clpoints:
        #cutter.edgeDrop(cl,cc,t)
        cutter.vertexDrop(cl,t)
        #cutter.facetDrop(cl,cc,t)
        #cutter.dropCutter(cl,cc,t)
        #ccpoints.append(cc)        
        n=n+1
        if (n % int(len(clpoints)/10)) == 0:
            print n/int(len(clpoints)/10), " ",
              
            
            
    print "done."
    
    print "rendering..."
    print " len(clpoints)=", len(clpoints)
    for cl in clpoints:
        myscreen.addActor( camvtk.Point(center=(cl.x,cl.y,cl.z) , color=camvtk.clColor(cl.cc)) ) 
    print "done."
        
    myscreen.camera.SetPosition(0.5, 3, 2)
    myscreen.camera.SetFocalPoint(0.5, 0.5, 0)
    myscreen.render()
    
    w2if = vtk.vtkWindowToImageFilter()
    w2if.SetInput(myscreen.renWin)
    lwr = vtk.vtkPNGWriter()
    lwr.SetInput( w2if.GetOutput() )
    
    t = camvtk.Text()
    t.SetPos( (myscreen.width-350, myscreen.height-30) )
    myscreen.addActor(t)
        
    for n in range(1,18):
        t.SetText("OpenCAMLib " + datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S"))
        myscreen.camera.Azimuth( 2 )
        time.sleep(0.1)
        myscreen.render()
        w2if.Modified()
        lwr.SetFileName("frames/tc"+ ('%04d' % n)+".png")
        #lwr.Write()



    myscreen.iren.Start()
    #raw_input("Press Enter to terminate") 
    
