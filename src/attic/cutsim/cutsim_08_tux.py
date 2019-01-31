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
    myscreen.camera.SetPosition(-8, -4, 25)
    myscreen.camera.SetFocalPoint(4.5,6, 0)   
    # axis arrows
    camvtk.drawArrows(myscreen,center=(-1,-1,0))
    camvtk.drawOCLtext(myscreen)
    
    octtext = camvtk.Text()
    octtext.SetPos( (70, myscreen.height-600) )
    myscreen.addActor( octtext)
    
    cltext = camvtk.Text()
    cltext.SetPos( (70, myscreen.height-100) )
    myscreen.addActor( cltext)
    
    
    stl = camvtk.STLSurf("../../stl/gnu_tux_mod.stl")
    #myscreen.addActor(stl)
    #stl.SetWireframe()
    stl.SetColor((0.5,0.5,0.5))
    
    polydata = stl.src.GetOutput()
    s = ocl.STLSurf()
    camvtk.vtkPolyData2OCLSTL(polydata, s)
    print("STL surface read,", s.size(), "triangles")
    
    #angle = math.pi/4
    radius  = 0.4
    length=10
    cutter = ocl.BallCutter(2*radius, length)
    #cutter = ocl.CylCutter(2*radius, length)
    
    # generate CL-points
    minx=0
    dx=0.1/0.2
    maxx=9
    miny=0
    dy=cutter.getRadius()/1.5
    maxy=12
    z=-1
    # this generates a list of CL-points in a grid
    clpoints = pyocl.CLPointGrid(minx,dx,maxx,miny,dy,maxy,z)
    # batchdropcutter    
    bdc = ocl.BatchDropCutter()
    bdc.bucketSize = 10
    bdc.setSTL(s)
    bdc.setCutter(cutter)
    #bdc.setThreads(1)  # explicitly setting one thread is better for debugging
    for p in clpoints:
        bdc.appendPoint(p)
    
    t_before = time.time()    
    bdc.run()
    
    t_after = time.time()
    calctime = t_after-t_before
    print(" BDC4 done in ", calctime," s")
    dropcutter_time = calctime
    clpoints = bdc.getCLPoints()
    
    #camvtk.drawCLPointCloud(myscreen, clpoints)
    print(" clpts= ", len(clpoints))
    myscreen.render()
    #myscreen.iren.Start() 
    #exit()
    
    s = ocl.BallCutterVolume()
    #s = ocl.CylCutterVolume()
    #s.center = ocl.Point(-2.50,-0.6,0)
    s.radius = cutter.getRadius()
    s.length = cutter.getLength()

    # screenshot writer
    w2if = vtk.vtkWindowToImageFilter()
    w2if.SetInput(myscreen.renWin)
    lwr = vtk.vtkPNGWriter()
    lwr.SetInput( w2if.GetOutput() )
    
    cp= ocl.Point(5,5,-6) # center of octree
    #depths = [3, 4, 5, 6, 7, 8]
    max_depth = 7
    root_scale = 10
    t = ocl.Octree(root_scale, max_depth, cp)
    t.init(5)
    n = 0 # the frame number
    
    stockbox = ocl.PlaneVolume( 1, 0, 0.1)
    t.diff_negative(stockbox)
    stockbox = ocl.PlaneVolume( 0, 0, 8.9  )
    t.diff_negative(stockbox)
    
    stockbox = ocl.PlaneVolume( 1, 1, 0.1)
    t.diff_negative(stockbox)
    stockbox = ocl.PlaneVolume( 0, 1, 11.9  )
    t.diff_negative(stockbox)
    
    stockbox = ocl.PlaneVolume( 1, 2, -0.5  )
    t.diff_negative(stockbox)
    stockbox = ocl.PlaneVolume( 0, 2, 3)
    t.diff_negative(stockbox)
    
    mc = ocl.MarchingCubes()
    
    print("stock mc()...",)
    tris = mc.mc_tree(t) # t.mc_triangles()
    print(" mc() got ", len(tris), " triangles")
    mc_surf = camvtk.STLSurf( triangleList=tris, color=camvtk.red )
    mc_surf.SetColor(camvtk.cyan)
    print("stock STLSurf()...",)
    myscreen.addActor( mc_surf )
    print("done.")
            
    myscreen.render()
    #myscreen.iren.Start() 
    #exit()
    myscreen.removeActor( mc_surf )
    renderinterleave=10
    step_time = 0
    while (n<len(clpoints)):
        cl = ocl.Point( clpoints[n].x, clpoints[n].y, clpoints[n].z )
        s.setPos( cl )
        #myscreen.addActor( camvtk.Point( center=(cl.x,cl.y,cl.z), color=camvtk.yellow))
        print(n,": diff...",)
        t_before = time.time() 
        t.diff_negative(s)
        t_after = time.time() 
        build_time = t_after-t_before
        #print "done in ", build_time," s"
        step_time=step_time+build_time
        n=n+1
        if ( (n%renderinterleave)==0):
            infotext= "Octree max_depth=%i \nCL-point %i of %i \ndiff()-time: %f ms/CL-point" % (max_depth,n, 
                                                                                                      len(clpoints), 1e3*step_time/renderinterleave )
            octtext.SetText(infotext)
            postext= "X: %f\nY: %f\nZ: %f" % (cl.x,cl.y,cl.z )
            cltext.SetText(postext)
            
            cactors = camvtk.drawBallCutter(myscreen, cutter, cl)
            print(cactors)
            t_before = time.time() 
            print("mc()...",)
            tris = mc.mc_tree(t) #.mc_triangles()
            t_after = time.time() 
            mc_time = t_after-t_before
            print("done in ", mc_time," s")
            print(" mc() got ", len(tris), " triangles")
            mc_surf = camvtk.STLSurf( triangleList=tris, color=camvtk.red )
            #mc_surf.SetWireframe()
            mc_surf.SetColor(camvtk.cyan)
            print(" STLSurf()...",)
            myscreen.addActor( mc_surf )
            print("done.")

            print(" render()...",)
            myscreen.render()
            myscreen.camera.Azimuth( 0.5 )
            lwr.SetFileName("frames/cutsim_d9_frame"+ ('%06d' % n)+".png")
            w2if.Modified() 
            lwr.Write()
            
            print("done.")
            myscreen.removeActor( mc_surf )
            for c in cactors:
                myscreen.removeActor( c )
            step_time = 0
        
        
        
        #lwr.SetFileName("frames/mc8_frame"+ ('%06d' % n)+".png")
        #myscreen.camera.Azimuth( 2 )
        #myscreen.render()
        #w2if.Modified() 
        #lwr.Write()
            
        #mc_surf.SetWireframe()
        #print "sleep...",
        #time.sleep(1.02)
        #print "done."
                
        

        # move forward
        #theta = n*dtheta
        #sp1 = ocl.Point(s.center)
        #s.center =  ocl.Point( 1.7*math.cos(theta),1.3*math.sin(theta),thetalift*theta)  
        #sp2 = ocl.Point(s.center)
        #print "line from ",sp1," to ",sp2
        #if n is not nmax:
        #    myscreen.addActor( camvtk.Line( p1=(sp1.x,sp1.y,sp1.z),p2=(sp2.x,sp2.y,sp2.z), color=camvtk.red ) )
        #print "center moved to", s.center
    
    print(" clpts= ", len(clpoints))
    print("All done.")
    myscreen.iren.Start() 

if __name__ == "__main__":

    main()
