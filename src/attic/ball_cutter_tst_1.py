import ocl 
import camvtk
import time
import vtk
  
if __name__ == "__main__":  
    myscreen = camvtk.VTKScreen()
    
    a = ocl.Point(1,0,0)
    myscreen.addActor(camvtk.Point(center=(a.x,a.y,a.z), color=(1,0,1)));
    b = ocl.Point(0,1,0)    
    myscreen.addActor(camvtk.Point(center=(b.x,b.y,b.z), color=(1,0,1)));
    c = ocl.Point(0,0,0.3)
    myscreen.addActor( camvtk.Point(center=(c.x,c.y,c.z), color=(1,0,1)));
    myscreen.addActor( camvtk.Line(p1=(a.x,a.y,a.z),p2=(c.x,c.y,c.z)) )
    myscreen.addActor( camvtk.Line(p1=(c.x,c.y,c.z),p2=(b.x,b.y,b.z)) )
    myscreen.addActor( camvtk.Line(p1=(a.x,a.y,a.z),p2=(b.x,b.y,b.z)) )
    t = ocl.Triangle(a,b,c)
    
    cutter = ocl.BullCutter(1,0.2)
    #cutter = ocl.CylCutter(0.5)
    #cutter = ocl.BallCutter(1.5)
    
    print(ocl.revision())
    print(cutter)
    
    
    
    print("rendering...",)
 
    # insert code here to actually do something...
                           
    print("done.")
    

    
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
    
