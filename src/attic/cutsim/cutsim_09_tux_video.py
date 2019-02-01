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
    length=5
    cutter = ocl.BallCutter(2*radius, length)
    #cutter = ocl.CylCutter(2*radius, length)
    
    
    
    # generate CL-points
    minx=0
    dx=0.1/0.4
    maxx=9
    miny=0
    dy=cutter.getRadius()/1.5
    maxy=12
    z=-1
    # this generates a list of CL-points in a grid
    clpoints = pyocl.CLPointGrid(minx,dx,maxx,miny,dy,maxy,z)
    # batchdropcutter    
    bdc = ocl.BatchDropCutter()
    bdc.bucketSize = 7
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
    
    cp= ocl.Point(5,5,-3) # center of octree
    #depths = [3, 4, 5, 6, 7, 8]
    max_depth = 7
    root_scale = 7
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
    
    print("mc()...",)
    tris = mc.mc_tree(t) #.mc_triangles()
    print(" mc() got ", len(tris), " triangles")
    mc_surf = camvtk.STLSurf( triangleList=tris, color=camvtk.red )
    mc_surf.SetColor(camvtk.cyan)
    print(" STLSurf()...",)
    myscreen.addActor( mc_surf )
    print("done.")
    cl = ocl.Point(0,0,5)
    cactors = camvtk.drawBallCutter(myscreen, cutter, cl)
    myscreen.render()
    #myscreen.iren.Start() 
    #exit()
    myscreen.removeActor( mc_surf )
    renderinterleave=len(clpoints)/100
    step_time = 0
    #render_time = 0
    while (n<len(clpoints)):
        cl = ocl.Point( clpoints[n].x, clpoints[n].y, clpoints[n].z )
        s.setPos( cl ) # move the cutter
        t_before = time.time() 
        t.diff_negative(s) # subtract cutter from stock
        t_after = time.time() 
        build_time = t_after-t_before
        step_time=step_time+build_time
        
        n=n+1
        if n<(len(clpoints)-renderinterleave):
            myscreen.removeActor( mc_surf )
            for c in cactors:
                myscreen.removeActor( c )
        if ( (n%renderinterleave)==0):

            
            cactors = camvtk.drawBallCutter(myscreen, cutter, cl)
            t_before = time.time() 
            print("mc()...",)
            tris = mc.mc_tree(t) #.mc_triangles()
            mc_time = time.time()-t_before
            print("done in ", mc_time," s")
            print(" mc() got ", len(tris), " triangles")
            print(" STLSurf()...",)
            
            
            t_before = time.time() 
            mc_surf = camvtk.STLSurf( triangleList=tris, color=camvtk.red )
            #mc_surf.SetWireframe()
            mc_surf.SetColor(camvtk.cyan)
            myscreen.addActor( mc_surf )
            print("done.")
            print(" render()...",)
            myscreen.render()
            render_time = time.time()-t_before
            myscreen.camera.Azimuth( 0.1 )
            lwr.SetFileName("frames/cutsim_d10_frame"+ ('%06d' % n)+".png")
            w2if.Modified() 
            
            call_ms = step_time/renderinterleave
            print(renderinterleave," diff() calls in", step_time, " = ", call_ms," ms/call")
            infotext= "Octree max_depth=%i \nCL-point %i of %i \n%i CL-pts/frame\ndiff()-time:  %1.3f s/CL-point\nmc()-time:  %1.3f s/frame\nrender()-time:  %1.3f s/frame\n%i Triangles" % (max_depth,n, 
                                                                                                      len(clpoints), renderinterleave, call_ms, mc_time, render_time, len(tris))
            octtext.SetText(infotext)
            postext= "X: %f\nY: %f\nZ: %f" % (cl.x,cl.y,cl.z )
            cltext.SetText(postext)
            
            #lwr.Write() # uncomment to actually write files to disk
            
            print("done.")

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

    
    print(" clpts= ", len(clpoints))
    print("All done.")
    myscreen.iren.Start() 

if __name__ == "__main__":

    main()
