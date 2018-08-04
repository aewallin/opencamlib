import ocl
import camvtk
import issue20data

if __name__ == "__main__":  
    s = ocl.STLSurf()
    triangles = issue20data.trilist
    
    for t in triangles:
        s.addTriangle(t)
    
    print ocl.version()    
    # define a cutter
    length = 10
    cutter = ocl.CylCutter(3, length)
    #cutter = ocl.BallCutter(3, length)
    #cutter = ocl.BullCutter(3,0.5, length)

    pdf = ocl.PathDropCutter()   # create a pdf-object for the surface s
    pdf.setSTL(s)
    pdf.setCutter(cutter)               # set the cutter
    pdf.minimumZ = -1                   # set the minimum Z-coordinate, or
                                        # "floor" for drop-cutter
                                        
    path = ocl.Path()                   # create an empty path object 
    # add a line to the path
    path.append(ocl.Line(ocl.Point(0, 0.098, 0), ocl.Point(4, 0.098, 0)))

    # set the path for pdf
    pdf.setPath( path )
    
    pdf.run()                   # run drop-cutter on the path
    
    clp = pdf.getCLPoints()     # get the cl-points from pdf


    print 'first point ', clp[0], '   ( z should be at z5 )'
    
    # visualize things with VTK
    myscreen = camvtk.VTKScreen()
    myscreen.addActor( camvtk.PointCloud( pointlist=clp ) )
    #myscreen.addActor( camvtk.PointCloud( pointlist=ccp, collist=ccp ) )
    su = camvtk.STLSurf(filename=None, triangleList=triangles )
    su.SetWireframe()
    myscreen.addActor(su)
    myscreen.render()
    myscreen.iren.Start()
