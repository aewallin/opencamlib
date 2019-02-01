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
    # axis arrows
    camvtk.drawArrows(myscreen,center=(-2,-2,0))
    camvtk.drawOCLtext(myscreen)
    
    
    s = ocl.BallCutterVolume()
    #s = ocl.CylCutterVolume()
    #s = ocl.BullCutterVolume()
    #s.center = ocl.Point(-2.50,-0.6,0)
    s.r1=0.3
    s.r2=0.1
    s.radius = 0.4
    s.length = 2
    startpoint = ocl.Point(0.46,1.0,0.4)
    s.setPos( startpoint )

    # screenshot writer
    w2if = vtk.vtkWindowToImageFilter()
    w2if.SetInput(myscreen.renWin)
    lwr = vtk.vtkPNGWriter()
    lwr.SetInput( w2if.GetOutput() )
    
    cp= ocl.Point(0,0,0) # center of octree
    max_depth = 6
    root_scale = 1
    t = ocl.Octree(root_scale, max_depth, cp)
    
    t.init(2)
    n = 0 # the frame number
    
    print("root_scale = ", t.root_scale())
    print(" max_depth = ", t.max_depth())
    print(" leaf_scale=", t.leaf_scale())
    
    # X
    #stockbox = ocl.PlaneVolume( 1, 0, -0.9)
    #t.diff_negative(stockbox)
    #stockbox = ocl.PlaneVolume( 0, 0, 0.9  )
    #t.diff_negative(stockbox)
    
    # Y
    #stockbox = ocl.PlaneVolume( 1, 1, -0.9)
    #t.diff_negative(stockbox)
    #stockbox = ocl.PlaneVolume( 0, 1, 0.9  )
    #t.diff_negative(stockbox)
    
    # Z
    #stockbox = ocl.PlaneVolume( 1, 2, 0.1  )
    #t.diff_negative(stockbox)
    #stockbox = ocl.PlaneVolume( 0, 2, 0.8)
    #t.diff_negative(stockbox)
    
    
    #t.diff_negative(s)
    
    mc = ocl.MarchingCubes()
    
    print("mc()...",)
    tris = mc.mc_tree(t) # t.mc_triangles()
    print(" mc() got ", len(tris), " triangles")
    #tris2 = t.side_triangles()
    #print "appending"
    #for tr in tris2:
    #    tris.append(tr)
    #print " side_triangles() got ", len(tris2), " triangles"
    mc_surf = camvtk.STLSurf( triangleList=tris )
    mc_surf.SetColor(camvtk.cyan)
    #s_surf = camvtk.STLSurf( triangleList=tris2 )
    #s_surf.SetColor(camvtk.yellow)
    
    #mc_surf.SetWireframe()
    #mc_surf.SetOpacity(0.3)
    
    print(" STLSurf()...",)
    myscreen.addActor( mc_surf )
    #myscreen.addActor( s_surf )
    print("done.")
    myscreen.render()
    
    
    myscreen.render()
    #myscreen.iren.Start() 
    #exit()
    myscreen.removeActor( mc_surf )
    #myscreen.removeActor( s_surf )
    #renderinterleave=900
    #step_time = 0
    Nmax=10
    #dy = float(-2)/float(Nmax)
    dy = - 2* t.leaf_scale()
    cl = startpoint
    while (n<Nmax):
        cl = cl + ocl.Point(0.0,dy,0)
        #cl = ocl.Point( clpoints[n].x, clpoints[n].y, clpoints[n].z )
        s.setPos( cl ) # move the cutter
        t_before = time.time() 
        t.diff_negative(s) # subtract cutter from stock
        t_after = time.time() 
        build_time = t_after-t_before
        #print n," diff() took ",build_time," s"
        #step_time=step_time+build_time
        if n<Nmax:
            myscreen.removeActor( mc_surf )
            #myscreen.removeActor( s_surf )
            #for c in cactors:
            #    myscreen.removeActor( c )
        
        #call_ms = 1e3*step_time/renderinterleave
        #print renderinterleave," diff() calls in", step_time, " = ", call_ms," ms/call"
        #infotext= "Octree max_depth=%i \nCL-point %i of %i \ndiff() CPU-time: %f ms/CL-point" % (max_depth,n, 
        #                                                                                          len(clpoints), call_ms )
        #octtext.SetText(infotext)
        #postext= "X: %f\nY: %f\nZ: %f" % (cl.x,cl.y,cl.z )
        #cltext.SetText(postext)
        
        #cactors = camvtk.drawBallCutter(myscreen, cutter, cl)
        #t_before = time.time() 
        #print "mc()...",
        tris = mc.mc_tree(t) #t.mc_triangles()
        #tris2 = t.side_triangles()
        #print "appending"
        #for tr in tris2:
        #    tris.append(tr)
        #mc_time = time.time()-t_before
        #print "done in ", mc_time," s"
        #print " mc() got ", len(tris), " triangles"
        #print " STLSurf()...",
        mc_surf = camvtk.STLSurf( triangleList=tris )
        mc_surf.SetWireframe()
        mc_surf.SetColor(camvtk.cyan)
        myscreen.addActor( mc_surf )
        #s_surf = camvtk.STLSurf( triangleList=tris2 )
        #s_surf.SetWireframe()
        #s_surf.SetColor(camvtk.yellow)
        #myscreen.addActor( s_surf )
        
        #print "done."
        #print " render()...",
        myscreen.render()
        #myscreen.camera.Azimuth( 0.1 )
        #lwr.SetFileName("frames/wireframe3_d8_frame"+ ('%06d' % n)+".png")
        #w2if.Modified() 
        #lwr.Write()
        
        #print "done."   
        #time.sleep(0.4)
        print(n, " mc_tris=",len(tris))
        #," side_tris=",len(tris2)
        n=n+1
        #myscreen.camera.SetPosition(3*math.cos( 7*float(n)/(float(Nmax)) ), 3*math.sin( 7*float(n)/(float(Nmax)) ), 5)
        #myscreen.camera.Azimuth( math.sin( 5*float(n)/(float(Nmax)) ) )
    print("all done.")
    myscreen.iren.Start() 
    exit()


if __name__ == "__main__":

    main()
