import ocl
import pyocl
import camvtk
import time
import datetime
import vtk

def main():  
    print(ocl.revision())
    myscreen = camvtk.VTKScreen()   
    myscreen.camera.SetPosition(-15, -8, 15)
    myscreen.camera.SetFocalPoint(0,0, 0)   
    # axis arrows
    camvtk.drawArrows(myscreen,center=(0,0,0))

    s = ocl.SphereOCTVolume()
    s.center = ocl.Point(0,0,0)
    s.radius = 2.6345
    
    sphere = camvtk.Sphere( center=(s.center.x,s.center.y,s.center.z), radius=s.radius, color=camvtk.cyan)
    sphere.SetOpacity(0.1)
    myscreen.addActor( sphere );
    
    
    # screenshot writer
    w2if = vtk.vtkWindowToImageFilter()
    w2if.SetInput(myscreen.renWin)
    lwr = vtk.vtkPNGWriter()
    lwr.SetInput( w2if.GetOutput() )
    
    # text
    camvtk.drawOCLtext(myscreen)
    octtext = camvtk.Text()
    octtext.SetPos( (myscreen.width-400, myscreen.height-290) )
    myscreen.addActor( octtext)

    
    
    cp= ocl.Point(0,0,0)
    #depths = [3, 4, 5, 6, 7, 8]
    depths = [4, 5 ]
    root_scale = 3
    n = 0 # the frame number
    for max_depth in depths:
        t = ocl.Octree(root_scale, max_depth, cp)
        t.init(1)
        print("build...",)
        t_before = time.time() 
        t.diff_positive(s)    
        t_after = time.time() 
        build_time = t_after-t_before
        print("done.")
        tris = t.mc_triangles()
        infotext= "Octree + Marching-Cubes test\nmax octree-depth:%i \ntriangles: %i \nbuild() time: %f ms" % (max_depth, 
                                                          len(tris), build_time*1e3 )
        octtext.SetText(infotext)
        
        mc_surf = camvtk.STLSurf( triangleList=tris, color=camvtk.red )
        #
        myscreen.addActor( mc_surf )
        print(" render()...",)
        myscreen.render()
        print("done.")
        for m in range(0,180):
            # do a rotating animation
            lwr.SetFileName("frames/mc8_frame"+ ('%06d' % n)+".png")
            myscreen.camera.Azimuth( 2 )
            myscreen.render()
            w2if.Modified() 
            #lwr.Write()
            
            if m > 90:
                mc_surf.SetWireframe()
            time.sleep(0.02)
            
            n=n+1
        myscreen.removeActor( mc_surf )
        
    myscreen.iren.Start() 

if __name__ == "__main__":

    main()
