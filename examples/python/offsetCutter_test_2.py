import ocl
import camvtk
import time
import vtk
import datetime
import math

def CLPointGrid(minx,dx,maxx,miny,dy,maxy,z):
    plist = []
    xvalues = [round(minx+n*dx,2) for n in range(int(round((maxx-minx)/dx))+1) ]
    yvalues = [round(miny+n*dy,2) for n in range(int(round((maxy-miny)/dy))+1) ]
    for y in yvalues:
        for x in xvalues:
            plist.append( ocl.CLPoint(x,y,z) )
    return plist

def drawPoints(myscreen, clpoints, ccpoints):
    c=camvtk.PointCloud( pointlist=clpoints, collist=ccpoints) 
    c.SetPoints()
    myscreen.addActor(c )
        

if __name__ == "__main__":  
    myscreen = camvtk.VTKScreen()
    
    a=ocl.Point(1,0.6,0.1)
    myscreen.addActor(camvtk.Point(center=(a.x,a.y,a.z), color=(1,0,1)))
    b=ocl.Point(0,1,0)    
    myscreen.addActor(camvtk.Point(center=(b.x,b.y,b.z), color=(1,0,1)))
    c=ocl.Point(0,0,0.0)
    myscreen.addActor(camvtk.Point(center=(c.x,c.y,c.z), color=(1,0,1)))
    
    myscreen.addActor( camvtk.Line(p1=(a.x,a.y,a.z),p2=(c.x,c.y,c.z)) )
    myscreen.addActor( camvtk.Line(p1=(c.x,c.y,c.z),p2=(b.x,b.y,b.z)) )
    myscreen.addActor( camvtk.Line(p1=(a.x,a.y,a.z),p2=(b.x,b.y,b.z)) )
    
    t = ocl.Triangle(b,c,a)
    radius1=1
    angle = math.pi/4
    length=10
    #cutter = ocl.ConeCutter(0.37, angle)
    
    cutter = ocl.BallCutter(0.532, length)
    #cutter = ocl.CylCutter(0.3, length)
    #cutter = ocl.BullCutter(0.7,0.1, length)
    
    # these cutters do not have offsets yet (?)
    #cutter =  ocl.CylConeCutter(0.2,0.5,math.pi/9)
    #cutter = ocl.BallConeCutter(0.4,0.6,math.pi/9)
    #cutter = ocl.BullConeCutter(0.4,0.1,0.7,math.pi/6)
    #cutter = ocl.ConeConeCutter(0.4,math.pi/3,0.7,math.pi/6)
    #cutter = ocl.ConeCutter(0.4, math.pi/3)
    print(cutter)
    offset=0.1
    c2 = cutter.offsetCutter(offset)
    print(c2)
    
    minx=-0.5
    dx=0.0051
    maxx=1.5
    miny=-0.7
    dy=dx
    maxy=1.5
    z=-1.8
    clpoints = CLPointGrid(minx,dx,maxx,miny,dy,maxy,z)
    print(len(clpoints), "cl-points to evaluate")
    n=0
    ccpoints=[]
    cl2pts=[]
    for p in clpoints:
        cl2pts.append(ocl.CLPoint(p.x,p.y,p.z))
        
    for (cl,cl2) in zip(clpoints,cl2pts):
        
        #cutter.vertexDrop(cl,t)
        #cutter.edgeDrop(cl,t)
        #cutter.facetDrop(cl,t)
        #c2.vertexDrop(cl2,t)
        cutter.dropCutter(cl,t)        
        c2.dropCutter(cl2,t)
        n=n+1
        if (n % int(len(clpoints)/10)) == 0:
            print(n/int(len(clpoints)/10), " ",)
            
    print("done.")
    
    print("rendering...")
    print(" len(clpoints)=", len(clpoints))
    print(" len(ccl2pts)=", len(cl2pts))
    print("rendering clpoints...",)
    camvtk.drawCLPointCloud(myscreen, clpoints)
    print("done.")
    cl2ptsofs=[]
    for p in cl2pts:
        p.z = p.z + offset 
        cl2ptsofs.append(p)
    print("rendering offset clpoints...",)
    camvtk.drawCLPointCloud(myscreen, cl2ptsofs)
    print("done.")
    origo = camvtk.Sphere(center=(0,0,0) , radius=0.1, color=camvtk.blue) 
    origo.SetOpacity(0.2)
    myscreen.addActor( origo )
     
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
    myscreen.iren.Start()
    #raw_input("Press Enter to terminate") 
