import ocl
import camvtk
import time
import vtk
import datetime

if __name__ == "__main__":  
    myscreen = camvtk.VTKScreen()
    
    stl = camvtk.STLSurf("../stl/demo.stl")
    print("STL surface read")
    myscreen.addActor(stl)
    stl.SetWireframe()
    stl.SetColor((0.5,0.5,0.5))
    
    polydata = stl.src.GetOutput()
    s= ocl.STLSurf()
    camvtk.vtkPolyData2OCLSTL(polydata, s)
    print("STLSurf with ", s.size(), " triangles")
    
    cutter = ocl.CylCutter(0.6)
    #print cutter.str()

    minx=-1
    dx=0.1
    maxx=11
    
    miny=-1
    dy=1
    maxy=11
    z=-0.2
    
    pdf = ocl.PathDropCutter(s)
    pdf.SetCutter(cutter)
    
    path = ocl.Path()
    
    
    exit()
    pftp = cam.ParallelFinish()
    pftp.initCLPoints(minx,dx,maxx,miny,dy,maxy,z)
    pftp.dropCutterSTL1(cutter, s) 
    print(" made ", pftp.dcCalls, " drop-cutter calls")
    
    pf2 = cam.ParallelFinish()
    pf2.initCLPoints(minx,dx,maxx,miny,dy,maxy,z)
    pf2.dropCutterSTL2(cutter, s) 
    print(" made ", pf2.dcCalls, " drop-cutter calls")
    
    clpoints = pftp.getCLPoints()
    ccpoints = pftp.getCCPoints()
    
    cl2p = pf2.getCLPoints()
    cc2p = pf2.getCCPoints()
    
    #CLPointGrid(minx,dx,maxx,miny,dy,maxy,z)
    nv=0
    nn=0
    ne=0
    nf=0
    for cl,cc,cl2 in zip(clpoints,ccpoints,cl2p):
        #cutter.dropCutter(cl,cc,t)
        #cc = cam.CCPoint()
        #cutter.dropCutterSTL(cl,cc,s)
        #    cutter.vertexDrop(cl,cc,t)
        #    cutter.edgeDrop(cl,cc,t)
        #    cutter.facetDrop(cl,cc,t)
        
        if cc.type==cam.CCType.FACET:
            nf+=1
            col = (0,1,1)
        elif cc.type == cam.CCType.VERTEX:
            nv+=1
            col = (0,1,0)
        elif cc.type == cam.CCType.EDGE:
            ne+=1
            col = (1,0,0)
        elif cc.type == cam.CCType.NONE:
            #print "type=NONE!"
            nn+=1
            col = (1,1,1)
        #if cl.isInside(t):
        #    col = (0, 1, 0)
        #else:
        #    col = (1, 0, 0)
               
        myscreen.addActor( camvtk.Point(center=(cl.x,cl.y,cl.z) , color=col) )    
        myscreen.addActor( camvtk.Point(center=(cl2.x,cl2.y,cl2.z+0.2) , color=(0.6,0.2,0.9)) )  
        #myscreen.addActor( camvtk.Point(center=(cc.x,cc.y,cc.z), color=col) )
        #print cc.type
        
    print("none=",nn," vertex=",nv, " edge=",ne, " facet=",nf, " sum=", nn+nv+ne+nf)
    print(len(clpoints), " cl points evaluated")
    myscreen.camera.SetPosition(3, 23, 15)
    myscreen.camera.SetFocalPoint(5, 5, 0)
    myscreen.render()
    
    w2if = vtk.vtkWindowToImageFilter()
    w2if.SetInput(myscreen.renWin)
    lwr = vtk.vtkPNGWriter()
    lwr.SetInput( w2if.GetOutput() )
    w2if.Modified()
    lwr.SetFileName("tux1.png")
    #lwr.Write()
    
    t = camvtk.Text()
    t.SetPos( (myscreen.width-200, myscreen.height-30) )
    myscreen.addActor( t)
     
    for n in range(1,36):
        t.SetText(datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S"))
        myscreen.camera.Azimuth( 1 )
        time.sleep(0.01)
        myscreen.render()
        lwr.SetFileName("kd_frame"+ ('%03d' % n)+".png")
        w2if.Modified() 
        #lwr.Write()

        


    #myscreen.iren.Start()
    raw_input("Press Enter to terminate") 
    
