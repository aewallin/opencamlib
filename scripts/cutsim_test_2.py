import ocl
import pyocl
import camvtk
import time
import datetime
import vtk



def main(filename="frame/f.png"):  
    print ocl.revision()
          
    myscreen = camvtk.VTKScreen()   
    myscreen.camera.SetPosition(20, 12, 20)
    myscreen.camera.SetFocalPoint(0,0, 0)   
    # axis arrows
    camvtk.drawArrows(myscreen,center=(2,2,2))

    
    # screenshot writer
    w2if = vtk.vtkWindowToImageFilter()
    w2if.SetInput(myscreen.renWin)
    lwr = vtk.vtkPNGWriter()
    lwr.SetInput( w2if.GetOutput() )
        

    
    c = ocl.CylCutter(2) # cutter
    c.length = 3
    print "cutter length=", c.length
    
    # generate CL-points
    stl = camvtk.STLSurf("../stl/demo.stl")
    polydata = stl.src.GetOutput()
    s = ocl.STLSurf()
    camvtk.vtkPolyData2OCLSTL(polydata, s)
    print "STL surface read,", s.size(), "triangles"
    print s.getBounds()
    #exit()
    minx=0
    dx=0.1/1
    maxx=10
    miny=0
    dy=1
    maxy=10
    z=-17
    # this generates a list of CL-points in a grid
    clpoints = pyocl.CLPointGridZigZag(minx,dx,maxx,miny,dy,maxy,z)
    print "generated grid with", len(clpoints)," CL-points"
    # batchdropcutter    
    bdc = ocl.BatchDropCutter()
    bdc.setSTL(s,1)
    bdc.setCutter(c)
    for p in clpoints:
        bdc.appendPoint(p)
    t_before = time.time()    
    print "threads=",bdc.nthreads
    bdc.dropCutter4()
    t_after = time.time()
    calctime = t_after-t_before
    print " done in ", calctime," s"
    clpoints = bdc.getCLPoints()
    
    # filter
    print "filtering. before filter we have", len(clpoints),"cl-points"
    f = ocl.LineCLFilter()
    f.setTolerance(0.001)
    for p in clpoints:
        f.addCLPoint(p)
    f.run()
    clpts = f.getCLPoints()
    print "after filtering we have", len(clpts),"cl-points"
    
    #exit()
        
    # stupid init code
    f=ocl.Ocode()
    f.set_depth(8) # depth and scale set here.
    f.set_scale(10)
    
    # cube
    stockvol = ocl.BoxOCTVolume()
    stockvol.corner = ocl.Point(0,0,-0.5)
    stockvol.v1 = ocl.Point(10,0,0)
    stockvol.v2 = ocl.Point(0,10,0)
    stockvol.v3 = ocl.Point(0,0,2)
    stockvol.calcBB()
    #cube1.side=10.0
    #cube1.center = ocl.Point(0,0,0)
    #cube1.calcBB()
    
    stock = ocl.LinOCT()
    stock.init(5)
    stock.build( stockvol )
    
    # draw initial octree 
    #tlist = pyocl.octree2trilist(stock)
    #surf = camvtk.STLSurf(triangleList=tlist)
    #myscreen.addActor(surf)
    
    # draw initial cutter
    #startp = ocl.Point(0,0,0)
    #cyl = camvtk.Cylinder(center=(startp.x,startp.y,startp.z), radius=c.radius,
    #                        height=c.length,
    #                        rotXYZ=(90,0,0), color=camvtk.grey)
    #cyl.SetWireframe()
    #myscreen.addActor(cyl)
    
    Nmoves = len(clpts)
    print Nmoves,"CL-points to process"
    for n in xrange(0,Nmoves-1):
        #if n<Nmoves-1:
        print n," to ",n+1
        startp = clpts[n]  # start of move
        endp   = clpts[n+1] # end of move
        sweep = ocl.LinOCT()
        sweep.init(5)
        g1vol = ocl.CylMoveOCTVolume(c, ocl.Point(startp.x,startp.y,startp.z), ocl.Point(endp.x,endp.y,endp.z))
        
        sweep.build( g1vol )
        # draw cutter
        cyl = camvtk.Cylinder(center=(startp.x,startp.y,startp.z), radius=c.radius,
                            height=c.length,
                            rotXYZ=(90,0,0), color=camvtk.grey)
        cyl.SetWireframe()
        myscreen.addActor(cyl)
    
        #camvtk.drawCylCutter(myscreen, c, startp)
        #camvtk.drawCylCutter(myscreen, c, endp)
        myscreen.addActor( camvtk.Line( p1=(startp.x,startp.y,startp.z), p2=(endp.x,endp.y,endp.z), color=camvtk.red))
        #camvtk.drawTree2(myscreen,sweep,color=camvtk.red,opacity=0.5)
            
        t_before = time.time()
        stock.diff(sweep)
        calctime = time.time()-t_before
        print " diff done in ", calctime," s"
    
        #myscreen.removeActor(surf)
        tlist = pyocl.octree2trilist(stock)
        surf = camvtk.STLSurf(triangleList=tlist)
        surf.SetColor(camvtk.cyan)
        surf.SetOpacity(1.0)
        myscreen.addActor(surf)
        myscreen.render()
        time.sleep(0.1)
        # write screenshot to disk
        lwr.SetFileName("cutsim_frame"+ ('%03d' % n)+".png")
        #lwr.SetFileName(filename)
        #lwr.Write()
        
        
        myscreen.removeActor(cyl)
        myscreen.render()
        #time.sleep(0.1)
        myscreen.removeActor(surf)
        #myscreen.render()
        #time.sleep(0.1)
        

            
    #exit()
    
    # draw trees
    #print "drawing trees"
    #camvtk.drawTree2(myscreen,  stock, opacity=1,   color=camvtk.cyan)        

    # box around octree 
    oct_cube = camvtk.Cube(center=(0,0,0), length=4*f.get_scale(), color=camvtk.white)
    oct_cube.SetWireframe()
    myscreen.addActor(oct_cube)
    
    # OCL text
    title = camvtk.Text()
    title.SetPos( (myscreen.width-350, myscreen.height-30) )
    title.SetText("OpenCAMLib " + datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S"))
    myscreen.addActor(title)
        
    print " render()...",
    myscreen.render()
    print "done."
    
    
    #time.sleep(0.2)
    
    
    
    myscreen.iren.Start() 

if __name__ == "__main__":

    main()
