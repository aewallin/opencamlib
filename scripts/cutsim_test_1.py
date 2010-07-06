import ocl
import camvtk
import time
import datetime
import vtk


    
def drawCylCutter(myscreen, c, p):
    cyl = camvtk.Cylinder(center=(p.x,p.y,p.z), radius=c.radius,
                            height=c.length,
                            rotXYZ=(90,0,0), color=camvtk.grey)
    cyl.SetWireframe()
    myscreen.addActor(cyl) 



def main(filename="frame/f.png"):  
    print ocl.revision()
          
    myscreen = camvtk.VTKScreen()   
    myscreen.camera.SetPosition(50, 22, 40)
    myscreen.camera.SetFocalPoint(0,0, 0)   
    # axis arrows
    camvtk.drawArrows(myscreen,center=(2,2,2))

    
    # screenshot writer
    w2if = vtk.vtkWindowToImageFilter()
    w2if.SetInput(myscreen.renWin)
    lwr = vtk.vtkPNGWriter()
    lwr.SetInput( w2if.GetOutput() )
        

    
    c = ocl.CylCutter(1) # cutter
    c.length = 3
    print "cutter length=", c.length
    p1 = ocl.CLPoint(-0.2,-0.2,0.2) # start of move
    p2 = ocl.CLPoint(1.5,1.5,-1) # end of move
    p3 = ocl.CLPoint(0,1.5,0.2)
    clpoints=[]
    clpoints.append(p1)
    clpoints.append(p3)
    clpoints.append(p2)
    
    f=ocl.Ocode()
    f.set_depth(6) # depth and scale set here.
    f.set_scale(1)
    
    # cube
    cube1 = ocl.CubeOCTVolume()
    cube1.side=2.123
    cube1.center = ocl.Point(0,0,0)
    cube1.calcBB()
    
    stock = ocl.LinOCT()
    stock.init(3)
    stock.build( cube1 )
    
    Nmoves = len(clpoints)
    print Nmoves,"CL-points to process"
    for n in xrange(0,Nmoves-1):
        #if n<Nmoves-1:
        print n," to ",n+1
        startp = clpoints[n]
        endp   = clpoints[n+1]
        sweep = ocl.LinOCT()
        sweep.init(3)
        g1vol = ocl.CylMoveOCTVolume(c, ocl.Point(startp.x,startp.y,startp.z), ocl.Point(endp.x,endp.y,endp.z))
        drawCylCutter(myscreen, c, startp)
        drawCylCutter(myscreen, c, endp)
        sweep.build( g1vol )
        stock.diff(sweep)
            
    #exit()
    
    # draw trees
    print "drawing trees"
    camvtk.drawTree2(myscreen,  stock, opacity=1,   color=camvtk.cyan)
    #camvtk.drawTree2(myscreen, t2, opacity=0.2, color=camvtk.cyan)
    #camvtk.drawTree2(myscreen, t2, opacity=1,   color=camvtk.cyan, offset=(5,0,0))
        

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
    
    
    lwr.SetFileName(filename)
    time.sleep(0.2)
    #lwr.Write()
    
    
    myscreen.iren.Start() 

if __name__ == "__main__":

    main()
        
        
        



