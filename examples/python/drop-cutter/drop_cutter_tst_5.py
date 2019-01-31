import ocl
import pyocl
import camvtk
import time
import vtk
import datetime

if __name__ == "__main__":  
    print(ocl.version())
    myscreen = camvtk.VTKScreen()
    stl = camvtk.STLSurf("../../stl/gnu_tux_mod.stl")
    
    myscreen.addActor(stl)
    stl.SetWireframe()
    stl.SetColor((0.5,0.5,0.5))

    polydata = stl.src.GetOutput()
    s= ocl.STLSurf()
    camvtk.vtkPolyData2OCLSTL(polydata, s)
    print("STL surface read,", s.size(), "triangles" )
    
    cutter = ocl.CylCutter(0.6, 6)
    print(cutter)

    minx=-0.2
    dx=1
    maxx=10.2
    miny=-0.2
    dy=1
    maxy=12.2
    z=-0.2
    clpoints = pyocl.CLPointGrid(minx,dx,maxx,miny,dy,maxy,z)
    print(len(clpoints),"CL-points to evaluate...")
    tris = s.getTriangles()
    # very naive approach to drop-cutter (and doing loops in python is slow)
    # this is so slow in practice it is not usable with the tux-model STL-file (22k triangles)
    for cl in clpoints: # loop through all the cl-points
        for t in tris: # for each cl-point loop through all triangles
            cutter.vertexDrop(cl,t) # vertex test
            cutter.edgeDrop(cl,t)  # edge test
            cutter.facetDrop(cl,t) # facet test
    print("done.")

    camvtk.drawCLPointCloud(myscreen, clpoints)

    print(len(clpoints), " cl points evaluated")
    myscreen.camera.SetPosition(3, 23, 15)
    myscreen.camera.SetFocalPoint(4, 5, 0)
    myscreen.render()
    
    #w2if = vtk.vtkWindowToImageFilter()
    #w2if.SetInput(myscreen.renWin)
    #lwr = vtk.vtkPNGWriter()
    #lwr.SetInput( w2if.GetOutput() )
    #w2if.Modified()
    #lwr.SetFileName("tux1.png")
    #lwr.Write()
    
    t = camvtk.Text()
    t.SetPos( (myscreen.width-200, myscreen.height-30) )
    myscreen.addActor( t)
     
    for n in range(1,36):
        t.SetText(datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S"))
        myscreen.camera.Azimuth( 1 )
        time.sleep(0.01)
        myscreen.render()
        #lwr.SetFileName("frame"+ ('%03d' % n)+".png")
        #w2if.Modified() 
        #lwr.Write() #uncomment to write screenshots to disk

    myscreen.iren.Start()
    raw_input("Press Enter to terminate") 
