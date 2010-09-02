import ocl
import pyocl
import camvtk
import time
import datetime
import vtk
import math

def main():  
    print ocl.revision()
    myscreen = camvtk.VTKScreen()   
    myscreen.camera.SetPosition(2, 2, 5)
    myscreen.camera.SetFocalPoint(0.5,0, 1)   
    camvtk.drawArrows(myscreen,center=(-2,-2,0)) # axis arrows
    camvtk.drawOCLtext(myscreen)
    
    #s = ocl.BallCutterVolume()
    #s = ocl.CylCutterVolume()
    s = ocl.BullCutterVolume()
    #s.center = ocl.Point(-2.50,-0.6,0)
    s.r1=0.3
    s.r2=0.1
    s.radius = 0.4
    s.length = 2
    startpoint = ocl.Point(0.46,1.0,0.4)
    s.setPos( startpoint )
    
    cp= ocl.Point(0,0,0) # center of octree
    max_depth = 6
    root_scale = 1
    t = ocl.Octree(root_scale, max_depth, cp)
    t.init(2)
    n = 0 # the frame number
    print "root_scale = ", t.root_scale()
    print " max_depth = ", t.max_depth()
    print " leaf_scale=", t.leaf_scale()

    print "mc()...",
    tris = t.mc_triangles()
    tris2 = t.side_triangles()
    mc_surf = camvtk.STLSurf( triangleList=tris )
    mc_surf.SetColor(camvtk.cyan)
    s_surf = camvtk.STLSurf( triangleList=tris2 )
    s_surf.SetColor(camvtk.yellow)
    print " STLSurf()...",
    myscreen.addActor( mc_surf )
    myscreen.addActor( s_surf )
    print "done."
    myscreen.render()
    myscreen.removeActor( mc_surf )
    myscreen.removeActor( s_surf )
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
        build_time = t_after-t_before
        if n<Nmax:
            myscreen.removeActor( mc_surf )
            myscreen.removeActor( s_surf )
        tris = t.mc_triangles()
        tris2 = t.side_triangles()
        mc_surf = camvtk.STLSurf( triangleList=tris )
        #mc_surf.SetWireframe()
        mc_surf.SetColor(camvtk.cyan)
        myscreen.addActor( mc_surf )
        s_surf = camvtk.STLSurf( triangleList=tris2 )
        s_surf.SetWireframe()
        s_surf.SetColor(camvtk.yellow)
        myscreen.addActor( s_surf )
        myscreen.render()
        print n, " mc_tris=",len(tris)," side_tris=",len(tris2)
        n=n+1

    print "all done."
    myscreen.iren.Start() 
    exit()

if __name__ == "__main__":
    main()
