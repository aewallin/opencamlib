import ocl as cam
import camvtk
import time
import vtk
import math
import datetime

    
    

def main():
    myscreen = camvtk.VTKScreen()
    focal = cam.Point(5, 5, 0)
    r = 30
    theta = (float(45)/360)*2*math.pi
    fi=45
    
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
        
    #w2if = vtk.vtkWindowToImageFilter()
    #w2if.SetInput(myscreen.renWin)
    #lwr = vtk.vtkPNGWriter()
    #lwr.SetInput( w2if.GetOutput() )
  
    
    t.SetText("OpenCAMLib 10.03-beta, " + datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S"))
    
    
    #ycoord = 1.1
    
    stl = camvtk.STLSurf(filename="../stl/demo.stl")
    #stl = camvtk.STLSurf(filename="../stl/demo2.stl")
    print "STL surface read"
    #myscreen.addActor(stl)
    #stl.SetWireframe()
    #stl.SetColor((0.5,0.5,0.5))
    polydata = stl.src.GetOutput()
    s= cam.STLSurf()
    camvtk.vtkPolyData2OCLSTL(polydata, s)
    print "STLSurf with ", s.size(), " triangles"
    
    
    myscreen.addActor( camvtk.Sphere( center=(0,0,0), radius=0.2, color = camvtk.yellow ) )
  
    s.build_kdtree()
    print "built kd-tree"
    s.jump_kd_reset()
    
    tlist = s.get_kd_triangles()
    
    print "got", len(tlist), " triangles"
    
    while (s.jump_kd_hi()):
        lotris = s.get_kd_triangles()
        s.jump_kd_up()
        cut = s.get_kd_cut()
        s.jump_kd_lo()
        hitris = s.get_kd_triangles()
        lev = s.get_kd_level()
        
        print "l=", lev, " hi=", len(hitris), " lo=", len(lotris), " cut=", cut
        
        if ( cut[0] < 2 ):
            print "x cut ",
            if ( cut[0] == 0):
                print "max" 
                myscreen.addActor( camvtk.Line( p1=(cut[1],100,0), p2=(cut[1],-100,0), color = camvtk.green ) )
            else:
                print "min" 
                myscreen.addActor( camvtk.Line( p1=(cut[1],100,0), p2=(cut[1],-100,0), color = camvtk.lgreen ) )
            #myscreen.addActor( camvtk.Line( p1=(100,cut[1],0), p2=(-100,cut[1],0), color = camvtk.red ) )
        else:
            print "y cut ",
            if ( cut[0] == 2):
                print "max" 
                myscreen.addActor( camvtk.Line( p1=(100,cut[1],0), p2=(-100,cut[1],0), color = camvtk.red ) )
            else:
                print "min"
                myscreen.addActor( camvtk.Line( p1=(100,cut[1],0), p2=(-100,cut[1],0), color = camvtk.pink ) )
            
        
        slo = camvtk.STLSurf(triangleList=lotris)
        slo.SetColor(camvtk.pink)
        slo.SetWireframe()
        shi = camvtk.STLSurf(triangleList=hitris)
        shi.SetColor(camvtk.lgreen)
        shi.SetWireframe()
        myscreen.addActor(slo)
        myscreen.addActor(shi)
        myscreen.render()
        myscreen.iren.Start()
        raw_input("Press Enter to terminate")   
        time.sleep(1)
        myscreen.removeActor(slo)
        myscreen.removeActor(shi)
    

  
    print "done."
    myscreen.render()
    #lwr.SetFileName(filename)
    
    #raw_input("Press Enter to terminate")         
    time.sleep(0.2)
    #lwr.Write()
    myscreen.iren.Start()


if __name__ == "__main__":
    main()
    #raw_input("Press Enter to terminate")    

    
