import ocl as cam
import camvtk
import time
import vtk

def CLPointGrid(minx,dx,maxx,miny,dy,maxy,z):
    """ generate and return a rectangular grid of points """
    plist = []
    xvalues = [round(minx+n*dx,2) for n in xrange(int(round((maxx-minx)/dx))+1) ]
    yvalues = [round(miny+n*dy,2) for n in xrange(int(round((maxy-miny)/dy))+1) ]
    for y in yvalues:
        for x in xvalues:
            plist.append( cam.Point(x,y,z) )
    return plist

def ccColor(cc):
    """ this function returns a different color depending on the type of
        the CC-point. Useful for visualizing CL or CC points """
    if cc.type==cam.CCType.FACET:
        col = (0,0,1)
    elif cc.type == cam.CCType.VERTEX:
        col = (0,1,0)
    elif cc.type == cam.CCType.EDGE:
        col = (1,0,0)
    elif cc.type == cam.CCType.NONE:
        col = (1,1,1)  
    elif cc.type == cam.CCType.ERROR:
        col = (0,1,1)
    return col
    
def ccColor2(cc):
    """ this function returns a different color depending on the type of
        the CC-point. Useful for visualizing CL or CC points """
    if cc.type==cam.CCType.FACET:
        col = (1,0,1)
    elif cc.type == cam.CCType.VERTEX:
        col = (1,1,0)
    elif cc.type == cam.CCType.EDGE:
        col = (0,1,1)
    elif cc.type == cam.CCType.NONE:
        col = (1,1,1)  
    elif cc.type == cam.CCType.ERROR:
        col = (0,0.5,1)
    return col    

if __name__ == "__main__":  
    myscreen = camvtk.VTKScreen()
    
    a=cam.Point(1,0,0)
    myscreen.addActor(camvtk.Point(center=(a.x,a.y,a.z), color=(1,0,1)));
    b=cam.Point(0,1,0)    
    myscreen.addActor(camvtk.Point(center=(b.x,b.y,b.z), color=(1,0,1)));
    c=cam.Point(0,0,0.3)
    myscreen.addActor(camvtk.Point(center=(c.x,c.y,c.z), color=(1,0,1)));
    #myscreen.addActor( camvtk.Line(p1=(a.x,a.y,a.z),p2=(c.x,c.y,c.z)) )
    #myscreen.addActor( camvtk.Line(p1=(c.x,c.y,c.z),p2=(b.x,b.y,b.z)) )
    #myscreen.addActor( camvtk.Line(p1=(a.x,a.y,a.z),p2=(b.x,b.y,b.z)) )
    t = cam.Triangle(a,b,c)
    
    #cutter = cam.BullCutter(1,0.2)
    #cutter = cam.CylCutter(0.5)
    cutter = cam.BallCutter(1.5)
    
    print cutter.str()
    
    
    #print cc.type
    minx=-0.7
    dx=0.04
    maxx=1.7
    miny=-0.7
    dy=0.06
    maxy=1.7
    z=-0.5
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
        #cutter.vertexDrop(cl,cc,t)
        #cutter.edgeDrop(cl,cc,t)
        #cutter.facetDrop(cl,cc,t)
        cutter.dropCutter(cl,cc,t)

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
    print "rendering...",
    for cl,cc in zip(clpoints,ccpoints):
        myscreen.addActor( camvtk.Point(center=(cl.x,cl.y,cl.z) , color=ccColor(cc) ) )
        if cc.type != cam.CCType.NONE: # only render interesting cc-points
            myscreen.addActor( camvtk.Point(center=(cc.x,cc.y,cc.z) , color=ccColor2(cc) ) )

                        
    print "done."
    
    #print "none=",nn," vertex=",nv, " edge=",ne, " facet=",nf, " sum=", nn+nv+ne+nf
    #print len(clpoints), " cl points evaluated"
    
    myscreen.camera.SetPosition(0.5, 3, 2)
    myscreen.camera.SetFocalPoint(0.5, 0.5, 0)
    myscreen.render()
    
    w2if = vtk.vtkWindowToImageFilter()
    w2if.SetInput(myscreen.renWin)
    lwr = vtk.vtkPNGWriter()
    lwr.SetInput( w2if.GetOutput() )
    w2if.Modified()
    lwr.SetFileName("5_all.png")
    #lwr.Write()



    myscreen.iren.Start()
    #raw_input("Press Enter to terminate") 
    
