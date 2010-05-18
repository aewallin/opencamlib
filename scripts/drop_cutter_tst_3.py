import ocl
import camvtk
import time

def CLPointGrid(minx,dx,maxx,miny,dy,maxy,z):
    plist = []
    xvalues = [round(minx+n*dx,2) for n in xrange(int(round((maxx-minx)/dx))+1) ]
    yvalues = [round(miny+n*dy,2) for n in xrange(int(round((maxy-miny)/dy))+1) ]
    for y in yvalues:
        for x in xvalues:
            plist.append( ocl.Point(x,y,z) )
    return plist

if __name__ == "__main__":  
    myscreen = camvtk.VTKScreen()
    
    a=ocl.Point(1,0,0)
    myscreen.addActor(camvtk.Point(center=(1,0,0), color=(1,1,1)));
    b=ocl.Point(0,1,0)    
    myscreen.addActor(camvtk.Point(center=(0,1,0), color=(1,1,1)));
    c=ocl.Point(0,0,0.2)
    myscreen.addActor( camvtk.Point(center=(0,0,0.2), color=(1,1,1)));
    myscreen.addActor( camvtk.Line(p1=(1,0,0),p2=(0,0,0.2)) )
    myscreen.addActor( camvtk.Line(p1=(0,0,0.2),p2=(0,1,0)) )
    myscreen.addActor( camvtk.Line(p1=(1,0,0),p2=(0,1,0)) )
    t = ocl.Triangle(a,b,c)
    
    cutter = ocl.CylCutter(.3)
    print cutter
    
    cc = ocl.CCPoint()

    minx=-0.2
    dx=0.02
    maxx=1.2
    miny=-0.2
    dy=0.2
    maxy=1.2
    z=-0.2
    clpoints = CLPointGrid(minx,dx,maxx,miny,dy,maxy,z)
    
    for cl in clpoints:
        cutter.dropCutter(cl,cc,t)
        
        if cc.type == ocl.CCType.FACET:
            col = (0,0,1)
        elif cc.type == ocl.CCType.VERTEX:
            col = (0,1,0)
        elif cc.type == ocl.CCType.EDGE:
            col = (1,0,0)
        elif cc.type == ocl.CCType.NONE:
            col = (1,1,1)
        
        myscreen.addActor( camvtk.Point(center=(cl.x,cl.y,cl.z) , color=col) )    
        myscreen.addActor( camvtk.Point(center=(cc.x,cc.y,cc.z), color=col) )
        #print cc.type
        
    print len(clpoints), " cl points evaluated"
    myscreen.iren.Start()
    #raw_input("Press Enter to terminate") 
    
