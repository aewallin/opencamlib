import ocl as cam
import camvtk
import time
import vtk
import datetime

if __name__ == "__main__":  
    myscreen = camvtk.VTKScreen()
    myscreen.setAmbient(1,1,1)
    
    #stl = camvtk.STLSurf(filename="demo.stl")
    stl = camvtk.STLSurf(filename="demo2.stl")
    print("STL surface read")
    myscreen.addActor(stl)
    stl.SetWireframe()
    stl.SetColor((0.5,0.5,0.8))
    #stl.SetFlat()
    polydata = stl.src.GetOutput()
    s= cam.STLSurf()
    camvtk.vtkPolyData2OCLSTL(polydata, s)
    print("STLSurf with ", s.size(), " triangles")
    cutterDiameter=20
    cutter = cam.CylCutter(cutterDiameter, 5)
    #print cutter.str()
    #print cc.type
    minx=20
    dx=15
    maxx=130
    
    miny=-70
    dy=1
    maxy=50
    z=-10
    
    bucketSize = 1
    
    #pftp = cam.ParallelFinish()
    #pftp.initCLPoints(minx,dx,maxx,miny,dy,maxy,z)
    #pftp.initSTLSurf(s, bucketSize)
    #pftp.dropCutterSTL1(cutter) 
    #print " made ", pftp.dcCalls, " drop-cutter calls"
    #exit
    
    pf2 = cam.ParallelFinish()
    pf2.initCLPoints(minx,dx,maxx,miny,dy,maxy,z)
    pf2.initSTLSurf(s, bucketSize)
    pf2.dropCutterSTL2(cutter) 
    print(" made ", pf2.dcCalls, " drop-cutter calls")
    
    #clpoints = pftp.getCLPoints()
    #ccpoints = pftp.getCCPoints()
    
    clpoints = pf2.getCLPoints()
    ccpoints = pf2.getCCPoints()
    
    #CLPointGrid(minx,dx,maxx,miny,dy,maxy,z)
    nv=0
    nn=0
    ne=0
    nf=0
    myscreen.camera.SetPosition(3, 300, 200)
    myscreen.camera.SetFocalPoint(75, 0, 0)
    t = camvtk.Text()
    t.SetPos( (myscreen.width-200, myscreen.height-30) )
    
    myscreen.addActor( t)
    
    t2 = camvtk.Text()
    t2.SetPos( (myscreen.width-200, 30) )
    myscreen.addActor( t2)
    t3 = camvtk.Text()
    t3.SetPos( (30, 30)) 
    myscreen.addActor( t3)
    
    t4 = camvtk.Text()
    t4.SetPos( (30, myscreen.height-60)) 
    myscreen.addActor( t4)
    
    n=0
    precl = cam.Point()
    
    w2if = vtk.vtkWindowToImageFilter()
    w2if.SetInput(myscreen.renWin)
    lwr = vtk.vtkPNGWriter()
    lwr.SetInput( w2if.GetOutput() )
    w2if.Modified()
    lwr.SetFileName("tux1.png")
    
    for cl,cc in zip(clpoints,ccpoints):
        
        camEye = myscreen.camera.GetFocalPoint()
        camPos = myscreen.camera.GetPosition()
        postext = "(%3.3f, %3.3f, %3.3f)" % (camPos[0], camPos[1], camPos[2])
        eyetext = "(%3.3f, %3.3f, %3.3f)" % (camEye[0], camEye[1], camEye[2])
        camtext = "Camera LookAt: "+eyetext+"\nCamera Pos: "+ postext
        t4.SetText(camtext)
        
        t.SetText(datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S"))
        xtext = "%3.3f" % cl.x
        ytext = "%3.3f" % cl.y
        ztext = "%3.3f" % cl.z
        t2.SetText( "X: " + xtext + "\nY: " + ytext + "\nZ: " + ztext )
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
        trilist = pf2.getTrianglesUnderCutter(cl, cutter)
        #print "at cl=", cl.str() , " where len(trilist)=", len(trilist)
        t3.SetText("Total Triangles: "+ str(s.size()) +"\nUnder Cutter (red): "+str(len(trilist)))
        
        stl2 = camvtk.STLSurf(filename=None, triangleList=trilist, color=(1,0,0)) # a new surface with only triangles under cutter
        stl2.SetWireframe()
        #stl2.SetFlat()
        myscreen.addActor(stl2)
        trilist=[]
        
        
        cutactor = camvtk.Cylinder(center=(cl.x,cl.y,cl.z), radius=cutterDiameter/2, height=20, color=(0.7,0,1))
        myscreen.addActor( cutactor )
        
        #myscreen.addActor( camvtk.Point(center=(cl.x,cl.y,cl.z) , color=col) )    
        
        if n==0:
            precl = cl
        else:
            d = cl-precl
            if (d.norm() < 90):
                myscreen.addActor( camvtk.Line( p1=(precl.x, precl.y, precl.z), p2=(cl.x, cl.y, cl.z), color=(0,1,1) ) )
            precl = cl
        n=n+1
        #myscreen.addActor( camvtk.Point(center=(cl2.x,cl2.y,cl2.z+0.2) , color=(0.6,0.2,0.9)) )  
        #myscreen.addActor( camvtk.Point(center=(cc.x,cc.y,cc.z), color=col) )
        #print cc.type
        myscreen.camera.Azimuth( 1 )
        #time.sleep(0.01)
        myscreen.render()
        w2if.Modified() 
        lwr.SetFileName("frames/kdbetter"+ ('%05d' % n)+".png")
        #lwr.Write()


        #raw_input("Press Enter to continue") 
        myscreen.removeActor(stl2)
        myscreen.removeActor( cutactor )
        
    print("none=",nn," vertex=",nv, " edge=",ne, " facet=",nf, " sum=", nn+nv+ne+nf)
    print(len(clpoints), " cl points evaluated")

    
    

    #lwr.Write()
    

     
    for n in range(1,36):
        t.SetText(datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S"))
        myscreen.camera.Azimuth( 1 )
        time.sleep(0.01)
        myscreen.render()
        lwr.SetFileName("kd_frame"+ ('%03d' % n)+".png")
        w2if.Modified() 
        #lwr.Write()

        


    myscreen.iren.Start()
    raw_input("Press Enter to terminate") 
    
