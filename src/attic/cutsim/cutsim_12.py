import ocl
import pyocl
import camvtk
import time
import datetime
import vtk
import math

def main():  
    print(ocl.revision())
    myscreen = camvtk.VTKScreen()   
    myscreen.camera.SetPosition(-15, -8, 15)
    myscreen.camera.SetFocalPoint(0,0, 0)   
    # axis arrows
    camvtk.drawArrows(myscreen,center=(0,0,0))

    s = ocl.SphereOCTVolume()
    s.center = ocl.Point(-2.50,-0.6,0)
    s.radius = 0.6345
    
    #sphere = camvtk.Sphere( center=(s.center.x,s.center.y,s.center.z), radius=s.radius, color=camvtk.cyan)
    #sphere.SetOpacity(0.1)
    #myscreen.addActor( sphere );
    
    
    # screenshot writer
    w2if = vtk.vtkWindowToImageFilter()
    w2if.SetInput(myscreen.renWin)
    lwr = vtk.vtkPNGWriter()
    lwr.SetInput( w2if.GetOutput() )
    
    # text
    #camvtk.drawOCLtext(myscreen)
    #octtext = camvtk.Text()
    #octtext.SetPos( (myscreen.width-400, myscreen.height-290) )
    #myscreen.addActor( octtext)

    
    
    cp= ocl.Point(0,0,-3)
    #depths = [3, 4, 5, 6, 7, 8]
    max_depth = 7
    root_scale = 3
    t = ocl.Octree(root_scale, max_depth, cp)
    t.init(4)
    n = 0 # the frame number
    nmax=200
    theta=0
    dtheta=0.1
    
    s.center =  ocl.Point( 2*math.cos(theta),1*math.sin(theta),0.01*theta)  
    
    mc = ocl.MarchingCubes()
   
    
    while (n<=nmax):
        print("diff...",)
        t_before = time.time() 
        t.diff_negative(s)
        t_after = time.time() 
        build_time = t_after-t_before
        print("done in ", build_time," s")
        #infotext= "Octree + Marching-Cubes test\nmax octree-depth:%i \ntriangles: %i \nbuild() time: %f ms" % (max_depth, 
        #                                                  len(tris), build_time*1e3 )
        #octtext.SetText(infotext)
        
        if n==nmax:
            t_before = time.time() 
            print("mc()...",)
            tris = mc.mc_tree(t) 
            t_after = time.time() 
            mc_time = t_after-t_before
            print("done in ", mc_time," s")
            print(" mc() got ", len(tris), " triangles")
            mc_surf = camvtk.STLSurf( triangleList=tris, color=camvtk.red )
            mc_surf.SetWireframe()
            mc_surf.SetColor(camvtk.cyan)
            print(" STLSurf()...",)
            myscreen.addActor( mc_surf )
            print("done.")
            print(" render()...",)
            myscreen.render()

            print("done.")

            #lwr.SetFileName("frames/mc8_frame"+ ('%06d' % n)+".png")
            #myscreen.camera.Azimuth( 2 )
            #myscreen.render()
            #w2if.Modified() 
            #lwr.Write()
                
            #mc_surf.SetWireframe()
            #print "sleep...",
            #time.sleep(1.02)
            #print "done."
                
            
            if n is not nmax:
                myscreen.removeActor( mc_surf )
                #myscreen.removeActor( oct_points )
        
        # move forward
        theta = n*dtheta
        s.center =  ocl.Point( 2*math.cos(theta),1*math.sin(theta),-0.1*theta)  
        print("center moved to", s.center)
        n=n+1
    print("All done.")
    myscreen.iren.Start() 

if __name__ == "__main__":

    main()
