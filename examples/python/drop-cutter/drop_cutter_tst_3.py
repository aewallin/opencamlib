import ocl
import pyocl
import camvtk
import time

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
    
    cutter = ocl.CylCutter(.3, 5)
    print(cutter)
    
    minx=-0.2
    dx=0.02
    maxx=1.2
    miny=-0.2
    dy=0.2
    maxy=1.2
    z=-0.2
    clpoints = pyocl.CLPointGrid(minx,dx,maxx,miny,dy,maxy,z)
    
    for cl in clpoints:
        cutter.dropCutter(cl,t)
                
    print(len(clpoints), " cl points evaluated")
    # draw the points
    camvtk.drawCLPointCloud(myscreen, clpoints)
    
    #position camera
    myscreen.camera.SetPosition(0.5, 3, 2)
    myscreen.camera.SetFocalPoint(0.5, 0.5, 0)
    
    myscreen.render()
    myscreen.iren.Start()
    #raw_input("Press Enter to terminate") 
    
