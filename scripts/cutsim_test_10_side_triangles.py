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
    myscreen.camera.SetPosition(-8, -4, 25)
    myscreen.camera.SetFocalPoint(4.5,6, 0)   
    # axis arrows
    camvtk.drawArrows(myscreen,center=(-1,-1,0))
    camvtk.drawOCLtext(myscreen)
    
    
    s = ocl.BallCutterVolume()
    #s = ocl.CylCutterVolume()
    #s.center = ocl.Point(-2.50,-0.6,0)
    s.radius = 1
    s.length = 2

    # screenshot writer
    w2if = vtk.vtkWindowToImageFilter()
    w2if.SetInput(myscreen.renWin)
    lwr = vtk.vtkPNGWriter()
    lwr.SetInput( w2if.GetOutput() )
    
    cp= ocl.Point(5,5,-3) # center of octree
    max_depth = 9
    root_scale = 7
    t = ocl.Octree(root_scale, max_depth, cp)
    n_div=5
    t.init(5)
    n = 0 # the frame number
    
    #stockbox = ocl.PlaneVolume( 1, 0, 0.1)
    #t.diff_negative(stockbox)
    #stockbox = ocl.PlaneVolume( 0, 0, 8.9  )
    #t.diff_negative(stockbox)
    
    #stockbox = ocl.PlaneVolume( 1, 1, 0.1)
    #t.diff_negative(stockbox)
    #stockbox = ocl.PlaneVolume( 0, 1, 11.9  )
    #t.diff_negative(stockbox)
    
    #stockbox = ocl.PlaneVolume( 1, 2, -0.5  )
    #t.diff_negative(stockbox)
    #stockbox = ocl.PlaneVolume( 0, 2, 3)
    #t.diff_negative(stockbox)
    
    
    print "mc()...",
    tris = t.mc_triangles()
    print " mc() got ", len(tris), " triangles"
    tris2 = t.side_triangles()
    for t in tris2:
        tris.append(t)
        
    print " side_triangles() got ", len(tris2), " triangles"
    
    mc_surf = camvtk.STLSurf( triangleList=tris, color=camvtk.red )
    mc_surf.SetColor(camvtk.cyan)
    mc_surf.SetWireframe()
    mc_surf.SetOpacity(0.3)
    
    print " STLSurf()...",
    myscreen.addActor( mc_surf )
    print "done."
    
    print "n_div=",n_div," so cube should have: ", 6*2*pow(2,2*n_div) ," trianlges"
    
    myscreen.render()
    myscreen.iren.Start() 
    exit()


if __name__ == "__main__":

    main()
