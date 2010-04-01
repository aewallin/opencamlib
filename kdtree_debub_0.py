import ocl as cam
import camvtk
import time
import vtk
import math
import datetime

    
    

def main(ycoord=1.2, filename="test", theta=60, fi=45):
    myscreen = camvtk.VTKScreen()
    focal = cam.Point(2.17, 1, 0)
    r = 14
    theta = (float(theta)/360)*2*math.pi
    
    
    campos = cam.Point( r*math.sin(theta)*math.cos(fi), r*math.sin(theta)*math.sin(fi), r*math.cos(theta) ) 
    myscreen.camera.SetPosition(campos.x, campos.y, campos.z)
    myscreen.camera.SetFocalPoint(focal.x,focal.y, focal.z)
    
    t = camvtk.Text()
    t.SetPos( (myscreen.width-450, myscreen.height-30) )

    
    myscreen.addActor( t)
    t2 = camvtk.Text()
    ytext = "kd-tree debug" #"Y: %3.3f" % (ycoord)
    t2.SetText(ytext)
    t2.SetPos( (50, myscreen.height-50) )   
    myscreen.addActor( t2)
        
    w2if = vtk.vtkWindowToImageFilter()
    w2if.SetInput(myscreen.renWin)
    lwr = vtk.vtkPNGWriter()
    lwr.SetInput( w2if.GetOutput() )
  
    epos = cam.Epos()
    epos.setS(0,1)

    t.SetText("OpenCAMLib 10.03-beta, " + datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S"))
    
    
    #ycoord = 1.1
    
    stl = camvtk.STLSurf(filename="stl/demo.stl")
    #stl = camvtk.STLSurf(filename="demo2.stl")
    print "STL surface read"
    myscreen.addActor(stl)
    stl.SetWireframe()
    stl.SetColor((0.5,0.5,0.5))
    polydata = stl.src.GetOutput()
    s= cam.STLSurf()
    camvtk.vtkPolyData2OCLSTL(polydata, s)
    print "STLSurf with ", s.size(), " triangles"
    
    
   
  
    s.build_kdtree()
    print "built kd-tree"
    s.jump_kd_reset()
    
    tlist = s.get_kd_triangles()
    
    print "got", len(tlist), " triangles"
    
    while (s.jump_kd_lo()):
        lotris = s.get_kd_triangles()
        s.jump_kd_up()
        s.jump_kd_hi()
        hitris = s.get_kd_triangles()
        lev = s.get_kd_level()
        print "l=", lev, " hi=", len(hitris), " lo=", len(lotris)
    

  
    print "done."
    myscreen.render()
    lwr.SetFileName(filename)
    
    #raw_input("Press Enter to terminate")         
    time.sleep(0.2)
    lwr.Write()
    myscreen.iren.Start()


if __name__ == "__main__":
    main()
    #raw_input("Press Enter to terminate")    

    
