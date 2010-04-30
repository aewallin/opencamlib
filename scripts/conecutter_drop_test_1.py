import ocl as cam
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
            plist.append( cam.Point(x,y,z) )
    return plist

def drawPoints(myscreen, clpoints, ccpoints):
    c=camvtk.PointCloud( pointlist=clpoints, collist=ccpoints) 
    c.SetPoints()
    myscreen.addActor(c )
        

if __name__ == "__main__":  
    myscreen = camvtk.VTKScreen()
    
    a=cam.Point(1,0,0.1)
    myscreen.addActor(camvtk.Point(center=(a.x,a.y,a.z), color=(1,0,1)))
    b=cam.Point(0,1,0)    
    myscreen.addActor(camvtk.Point(center=(b.x,b.y,b.z), color=(1,0,1)))
    c=cam.Point(0,0,0.4)
    myscreen.addActor(camvtk.Point(center=(c.x,c.y,c.z), color=(1,0,1)))
    
    myscreen.addActor( camvtk.Line(p1=(a.x,a.y,a.z),p2=(c.x,c.y,c.z)) )
    myscreen.addActor( camvtk.Line(p1=(c.x,c.y,c.z),p2=(b.x,b.y,b.z)) )
    myscreen.addActor( camvtk.Line(p1=(a.x,a.y,a.z),p2=(b.x,b.y,b.z)) )
    
    t = cam.Triangle(a,b,c)
    radius1=1
    angle = math.pi/4
    #cutter = cam.ConeCutter(0.37, angle)
    cutter = cam.BallCutter(0.7)
    #cutter = cam.CylCutter(0.3)
    #cutter = cam.BullCutter(0.7,0.15)
    print cutter.str()
    
    
    #print cc.type
    minx=-0.5
    dx=0.005
    maxx=1.5
    miny=-0.7
    dy=dx
    maxy=1.5
    z=-1.8
    clpoints = CLPointGrid(minx,dx,maxx,miny,dy,maxy,z)
    nv=0
    nn=0
    ne=0
    nf=0
    print len(clpoints), "cl-points to evaluate"
    n=0
    ccpoints=[]
    
    for cl in clpoints:

        cc = cam.CCPoint()
        
        cutter.vertexDrop(cl,cc,t)
        cutter.edgeDrop(cl,cc,t)
        
        cutter.facetDrop(cl,cc,t)
        
        #cutter.dropCutter(cl,cc,t)

        ccpoints.append(cc)
       
        #if (n % 2 == 0):
        #    myscreen.addActor( camvtk.Point(center=(cl.x,cl.y,cl.z) , color=col) ) 
        
        #myscreen.addActor( camvtk.Point(center=(cl.x,cl.y,cl.z) , color=col) )    
        #myscreen.addActor( camvtk.Point(center=(cc.x,cc.y,cc.z), color=col) )

        #print cc.type
        n=n+1
        if (n % int(len(clpoints)/10)) == 0:
            print n/int(len(clpoints)/10), " ",
              
            
            
    print "done."
    
    print "rendering..."
    print " len(clpoints)=", len(clpoints)
    print " len(ccpoints)=", len(ccpoints)
    
    drawPoints(myscreen, clpoints, ccpoints)
    
    """
    for cl,cc in zip(clpoints,ccpoints):
        myscreen.addActor( camvtk.Point(center=(cl.x,cl.y,cl.z) , color=camvtk.clColor(cc)) ) 
        
        if cc.type != cam.CCType.NONE:
            pass
            myscreen.addActor( camvtk.Point(center=(cc.x,cc.y,cc.z) , color=camvtk.ccColor(cc)) ) 
        
        if cc.type != cam.CCType.NONE and cc.x==0.0 and cc.y == 0.0 and cc.z == 0:
            print "error cl=", cl.str()
    """
            

    print "done."
    origo = camvtk.Sphere(center=(0,0,0) , radius=0.1, color=camvtk.blue) 
    origo.SetOpacity(0.2)
    myscreen.addActor( origo )
     
    #print "none=",nn," vertex=",nv, " edge=",ne, " facet=",nf, " sum=", nn+nv+ne+nf
    #print len(clpoints), " cl points evaluated"
    
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
    
    #t2 = camvtk.Text()
    #t2.SetPos( (50, myscreen.height-80) )
    #myscreen.addActor(t2)
    #cuttertext= "Toroidal cutter:\nr1=%f\nr2=%f" % (radius1,radius2)
    #t2.SetText(cuttertext)
    
    """
    for n in range(1,18):
        t.SetText("OpenCAMLib " + datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S"))
        myscreen.camera.Azimuth( 2 )
        time.sleep(0.1)
        myscreen.render()
        w2if.Modified()
        lwr.SetFileName("frames/tc"+ ('%04d' % n)+".png")
        #lwr.Write()
    """


    myscreen.iren.Start()
    #raw_input("Press Enter to terminate") 
    
