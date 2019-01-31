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
    myscreen.camera.SetPosition(2, 2, 5)
    myscreen.camera.SetFocalPoint(0.5,0, 1)   
    camvtk.drawArrows(myscreen,center=(-2,-2,0)) # axis arrows
    camvtk.drawOCLtext(myscreen)
    
    #s = ocl.BallCutterVolume()
    #s = ocl.CylCutterVolume()
    s = ocl.BullCutterVolume()
    s.r1=0.3
    s.r2=0.1
    s.radius = 0.4
    s.length = 2
    startpoint = ocl.Point(0.46,0.2,0.4)
    s.setPos( startpoint )
    cp= ocl.Point(0,0,0) # center of octree
    max_depth = 6
    root_scale = 1
    t = ocl.Octree(root_scale, max_depth, cp)
    t.init(2)
    n = 0 # the frame number
    print("root_scale = ", t.root_scale())
    print(" max_depth = ", t.max_depth())
    print(" leaf_scale=", t.leaf_scale())

    print("mc()...",)
    mc = ocl.MarchingCubes()
    tris = mc.mc_tree(t) 

    mc_surf = camvtk.STLSurf( triangleList=tris )
    mc_surf.SetColor(camvtk.cyan)

    print(" STLSurf()...",)
    myscreen.addActor( mc_surf )

    print("done.")
    myscreen.render()
    myscreen.removeActor( mc_surf )

    Nmax=20
    dy = - 1* t.leaf_scale()
    cl = startpoint
    renderinterleave=1
    while (n<Nmax):
        
        cl = cl + ocl.Point(0.0,dy,0)
        s.setPos( cl ) # move the cutter
        t_before = time.time() 
        t.diff_negative(s) # subtract cutter from stock
        t_after = time.time() 
        diff_time = t_after-t_before
        if n<Nmax:
            myscreen.removeActor( mc_surf ) # leave the surface on the very last iteration
            
        # marching cubes
        t_before = time.time() 
        tris = mc.mc_tree(t) 
        t_after = time.time()
        mc_time = t_after-t_before
        
        # render
        t_before = time.time()
        mc_surf = camvtk.STLSurf( triangleList=tris )
        mc_surf.SetColor(camvtk.cyan)
        myscreen.addActor( mc_surf )
        myscreen.render()
        
        t_after = time.time()
        render_time = t_after-t_before
        print("%i: mc_tris=%i\t diff=%1.3f \t mc=%1.3f \t ren=%1.3f" % (n, len(tris), diff_time, mc_time, render_time))
        n=n+1

    print("all done.")
    myscreen.iren.Start() 
    exit()

if __name__ == "__main__":
    main()
