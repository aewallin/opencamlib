import ocl
import camvtk
import time
import vtk
import datetime

if __name__ == "__main__":  
    myscreen = camvtk.VTKScreen()
    
    stl = camvtk.STLSurf("../stl/gnu_tux_mod.stl")
    print "STL surface read"
    myscreen.addActor(stl)
    stl.SetWireframe()
    stl.SetColor((0.5,0.5,0.5))
    
    polydata = stl.src.GetOutput()
    s= ocl.STLSurf()
    camvtk.vtkPolyData2OCLSTL(polydata, s)
    print "STLSurf with ", s.size(), " triangles"
    
    # define a cutter
    cutter = ocl.CylCutter(0.6)
    print cutter
    
    # define the area 
    minx=-1
    dx=0.1
    maxx=11
    
    miny=-1
    dy=1
    maxy=11
    z=-0.2
    
    # create a pdf-object for the surface s
    pdf = ocl.PathDropCutterFinish(s)
    # set the cutter
    pdf.setCutter(cutter)
    # set the minimum Z-coordinate, or "floor" for drop-cutter
    pdf.minimumZ = -1
    
    # create an empty object path
    path = ocl.Path()
    
    xmax=10
    xmin=0
    Nx = 10
    dx = float(xmax-xmin)/Nx
    
    ymin=0
    ymax=12
    Ny=40
    dy = float(ymax-ymin)/Ny
    
    for n in xrange(0,Ny):
        y = ymin+n*dy
        # start-point of line
        p1 = ocl.Point(0,y,0)
        # end poitn of line
        p2 = ocl.Point(9,y,0)
        # line-object
        l = ocl.Line(p1,p2)
        # add the line to the path
        path.append( l )

    # set the path for pdf
    pdf.setPath( path )
    
    t_before = time.time()
    # run drop-cutter on the path
    pdf.run()
    t_after = time.time()
    print "run took ", t_after-t_before," s"
    
    # get the cl-points from pdf
    clp = pdf.getCLPoints()
    # get the cc-points from pdf
    ccp = pdf.getCCPoints()
    
    print "calculated ", len(clp)," CL-points"
    # render the CL-points
    myscreen.addActor( camvtk.PointCloud(pointlist=clp, collist=ccp)  )
    
    myscreen.camera.SetPosition(3, 23, 15)
    myscreen.camera.SetFocalPoint(5, 5, 0)
    myscreen.render()
    myscreen.iren.Start()
