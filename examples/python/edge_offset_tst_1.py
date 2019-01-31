import ocl
import pyocl
import camvtk
import time
import vtk
import datetime
import math

def drawEdge(myscreen, a, b):
    myscreen.addActor(camvtk.Sphere(center=(a.x,a.y,a.z), radius=0.0351, color=camvtk.green));
    myscreen.addActor(camvtk.Sphere(center=(b.x,b.y,b.z), radius=0.0351, color=camvtk.red));
    myscreen.addActor( camvtk.Line(p1=(a.x,a.y,a.z),p2=(b.x,b.y,b.z)) )

if __name__ == "__main__":  
    print(ocl.version())
    myscreen = camvtk.VTKScreen()
    camvtk.drawOCLtext(myscreen)
    camvtk.drawArrows(myscreen,center=(-1,-2,0))
    a=ocl.Point(0,1.7,-0.6)
    b=ocl.Point(0,0.11,0.3)    

    drawEdge(myscreen, a, b)
    diameter=0.4
    length=1
    # spherical cutter and cylinder
    s1 = camvtk.Sphere(center=(a.x,a.y,a.z), radius=diameter/2, color=camvtk.lgreen)
    s2 = camvtk.Sphere(center=(b.x,b.y,b.z), radius=diameter/2, color=camvtk.pink)
    s1.SetOpacity(1)
    s2.SetOpacity(1)
    myscreen.addActor(s1)
    myscreen.addActor(s2)
    # tube
    cyltube = camvtk.Tube( p1=(a.x,a.y,a.z) , p2=(b.x,b.y,b.z), radius=diameter/2, color=camvtk.yellow )
    cyltube.SetOpacity(0.2)
    myscreen.addActor( cyltube )
    
    
    # Cylinder
    a = a + ocl.Point(1,0,0)
    b = b + ocl.Point(1,0,0)
    drawEdge(myscreen, a, b)
    cir1 = camvtk.Circle(center=(a.x,a.y,a.z) , radius=diameter/2, color=camvtk.lgreen, resolution=50 )
    cir1.SetOpacity(1)
    myscreen.addActor(cir1)
    cir2 = camvtk.Circle(center=(b.x,b.y,b.z) , radius=diameter/2, color=camvtk.pink, resolution=50 )
    cir2.SetOpacity(1)
    myscreen.addActor(cir2)
    
    # draw lines along the elliptic tube
    
    # Toroid
    a = a + ocl.Point(1,0,0)
    b = b + ocl.Point(1,0,0)
    drawEdge(myscreen, a, b)
    tor1 = camvtk.Toroid(r1=diameter/2, r2=diameter/6, center=(a.x,a.y,a.z), rotXYZ=(0,0,0), color=camvtk.lgreen)
    tor1.SetOpacity(1)
    myscreen.addActor(tor1)
    tor2 = camvtk.Toroid(r1=diameter/2, r2=diameter/6, center=(b.x,b.y,b.z), rotXYZ=(0,0,0), color=camvtk.pink)
    tor2.SetOpacity(1)
    myscreen.addActor(tor2)
    
    # Cone
    a = a + ocl.Point(1,0,0)
    b = b + ocl.Point(1,0,0)
    drawEdge(myscreen, a, b)
    con1 = camvtk.Cone(center=(a.x,a.y,a.z), radius=diameter/2,  height = 0.3, color=camvtk.lgreen )
    myscreen.addActor(con1)
    con2 = camvtk.Cone(center=(b.x,b.y,b.z), radius=diameter/2,  height = 0.3, color=camvtk.pink )
    myscreen.addActor(con2)
    
    
    print("done.")
        
    myscreen.camera.SetPosition(4, 3, 2)
    myscreen.camera.SetFocalPoint(0, 0, 0)
    myscreen.render()
    
    w2if = vtk.vtkWindowToImageFilter()
    w2if.SetInput(myscreen.renWin)
    lwr = vtk.vtkPNGWriter()
    lwr.SetInput( w2if.GetOutput() )

        
    #for n in range(1,18):
    #    t.SetText("OpenCAMLib " + datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S"))
    #    myscreen.camera.Azimuth( 2 )
    #    time.sleep(0.1)
    #    myscreen.render()
    #    w2if.Modified()
    #    lwr.SetFileName("frames/tc"+ ('%04d' % n)+".png")
        #lwr.Write()



    myscreen.iren.Start()
    #raw_input("Press Enter to terminate") 
    
