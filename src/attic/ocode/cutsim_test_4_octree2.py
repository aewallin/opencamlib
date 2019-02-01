import ocl
import pyocl
import camvtk
import time
import datetime
import vtk



def main(filename="frame/f.png"):  
    print(ocl.revision())
          
    myscreen = camvtk.VTKScreen()   
    myscreen.camera.SetPosition(-15, -8, 15)
    myscreen.camera.SetFocalPoint(5,5, 0)   
    # axis arrows
    camvtk.drawArrows(myscreen,center=(0,0,0))

    
    # screenshot writer
    w2if = vtk.vtkWindowToImageFilter()
    w2if.SetInput(myscreen.renWin)
    lwr = vtk.vtkPNGWriter()
    lwr.SetInput( w2if.GetOutput() )
        

    
    c = ocl.CylCutter(3,10) # cutter
    c.length = 3
    print("cutter length=", c.length)
    
    cp= ocl.Point(0,0,0)
    max_depth = 9
    root_scale = 3
    t = ocl.Octree(root_scale, max_depth, cp)
    print(t)
    
    nodes = t.get_leaf_nodes()
    t.init(1)
    #nodes=[]
    s = ocl.SphereOCTVolume()
    s.center = ocl.Point(0,0,0)
    s.radius = 2.6345
    print("build...",)
    t.build(s)
    print("done.")
    print(t)
    
    sphere = camvtk.Sphere( center=(s.center.x,s.center.y,s.center.z), radius=s.radius, color=camvtk.cyan)
    sphere.SetOpacity(0.1)
    myscreen.addActor( sphere );
    
    
    nodes = t.get_surface_nodes()
    print("got ", len(nodes)," surface nodes")
   
    points=[]
    for n in nodes:
        #n=nodes[0]
        verts = n.vertices()
        #c = n.center
        #print " node at depth=", n.depth," center=",c
        #myscreen.addActor( camvtk.Sphere( center=(c.x,c.y,c.z), radius=0.1, color=camvtk.yellow ))
        for v in verts:
            #print v
            #myscreen.addActor( camvtk.Sphere( center=(v.x,v.y,v.z), radius=0.1 ))
            #
            points.append(v)
            
    #myscreen.addActor( camvtk.PointCloud( pointlist= points))
    
    tris = t.mc_triangles()
    mc_surf = camvtk.STLSurf( triangleList=tris, color=camvtk.red )
    #mc_surf.SetWireframe()
    myscreen.addActor( mc_surf )
    print(" render()...",)
    myscreen.render()
    print("done.")
    
    
    #time.sleep(0.2)
    
    
    
    myscreen.iren.Start() 

if __name__ == "__main__":

    main()
