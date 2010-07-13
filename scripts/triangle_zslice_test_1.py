import ocl
import camvtk
import time
import vtk
import datetime
import math

if __name__ == "__main__":  
    myscreen = camvtk.VTKScreen()
    
    a = ocl.Point(0,1,0.3)
    myscreen.addActor(camvtk.Point(center=(a.x,a.y,a.z), color=(1,0,1)))
    b = ocl.Point(1,0.5,0)    
    myscreen.addActor(camvtk.Point(center=(b.x,b.y,b.z), color=(1,0,1)))
    c = ocl.Point(0,0,0)
    myscreen.addActor(camvtk.Point(center=(c.x,c.y,c.z), color=(1,0,1)))
    myscreen.addActor( camvtk.Line(p1=(a.x,a.y,a.z),p2=(c.x,c.y,c.z)) )
    myscreen.addActor( camvtk.Line(p1=(c.x,c.y,c.z),p2=(b.x,b.y,b.z)) )
    myscreen.addActor( camvtk.Line(p1=(a.x,a.y,a.z),p2=(b.x,b.y,b.z)) )
    t = ocl.Triangle(b,c,a)
    print t
    zcut=0.1
    print "zslice at z=",zcut
    t2 = t.zslice(zcut)
    for tri in t2:
        print tri
        pts = tri.getPoints()
        for p in pts:
            myscreen.addActor( camvtk.Sphere( center=(p.x, p.y, p.z), radius=0.02, color=camvtk.green))
        
    print "done."
    
    myscreen.camera.SetPosition(0.5, 3, 2)
    myscreen.camera.SetFocalPoint(0.5, 0.5, 0)
    
    camvtk.drawArrows(myscreen,center=(-0.5,-0.5,-0.5))

    camvtk.drawOCLtext(myscreen)
    
    
    myscreen.render()    
    w2if = vtk.vtkWindowToImageFilter()
    w2if.SetInput(myscreen.renWin)
    lwr = vtk.vtkPNGWriter()
    lwr.SetInput( w2if.GetOutput() )

    myscreen.iren.Start()
    #raw_input("Press Enter to terminate") 
