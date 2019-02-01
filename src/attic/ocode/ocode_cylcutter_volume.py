import ocl
import camvtk
import time
import datetime
import vtk



def drawTree(myscreen,t,color=camvtk.red,opacity=0.2, offset=(0,0,0)):
    nodes = t.get_nodes()
    nmax=len(nodes)
    i=0
    for n in nodes:
        cen = n.point()
        #print "cen=",cen.str()
        scale = n.get_scale()
        #print "col=", n.color
        

        cube = camvtk.Cube(center=(cen.x+offset[0], cen.y+offset[1], cen.z+offset[2]), length= scale, color=color)
        cube.SetOpacity(opacity)
        #cube.SetPhong()
        cube.SetGouraud()
        #cube.SetWireframe()
        myscreen.addActor( cube )
        if ( (i % (nmax/10))==0):
            print(".",)
        i=i+1
    print("done.")
    
def drawTree2(myscreen,t,color=camvtk.red,opacity=0.2, offset=(0,0,0)):
    nodes = t.get_nodes()
    nmax=len(nodes)
    print("drawTree2: ", nmax," nodes",)
    # make a list of triangles
    tlist = []
    i=0
    for n in nodes:
        p1 = n.corner(0) # + + +
        p2 = n.corner(1) # - + +
        p3 = n.corner(2) # + - +
        p4 = n.corner(3) # + + -
        p5 = n.corner(4) # + - -
        p6 = n.corner(5) # - + -
        p7 = n.corner(6) # - - +
        p8 = n.corner(7) # - - -
        tlist.append(ocl.Triangle(p1,p2,p3)) #top
        tlist.append(ocl.Triangle(p2,p3,p7)) #top
        
        tlist.append(ocl.Triangle(p4,p5,p6)) # bot
        tlist.append(ocl.Triangle(p5,p6,p8)) # bot
        
        tlist.append(ocl.Triangle(p1,p3,p4)) # 1,3,4,5
        tlist.append(ocl.Triangle(p4,p5,p3))
        
        tlist.append(ocl.Triangle(p2,p6,p7)) # 2,6,7,8
        tlist.append(ocl.Triangle(p7,p8,p6))
        
        tlist.append(ocl.Triangle(p3,p5,p7)) # 3,5,7,8
        tlist.append(ocl.Triangle(p7,p8,p5))
        
        tlist.append(ocl.Triangle(p1,p2,p4)) # 1,2,4,6
        tlist.append(ocl.Triangle(p4,p6,p2))
        if ( (i % (nmax/10))==0):
            print(".",)
        i=i+1
            
        #tlist.append(ocl.Triangle(p1,p2,p4))
        #tlist.append(ocl.Triangle(p1,p3,p5))
        #tlist.append(ocl.Triangle(p2,p3,p7))
        #tlist.append(ocl.Triangle(p2,p7,p8))
        #tlist.append(ocl.Triangle(p3,p7,p8))
    print("done")
    surf = camvtk.STLSurf(triangleList=tlist)
    surf.SetColor(color)
    surf.SetOpacity(opacity)
    myscreen.addActor(surf)
    

def drawBB( myscreen, vol ):
    lines = []
    lines.append( camvtk.Line(p1=(vol.bb.minx, vol.bb.miny, vol.bb.minz) , p2=(vol.bb.maxx, vol.bb.miny, vol.bb.minz))  )
    lines.append( camvtk.Line(p1=(vol.bb.minx, vol.bb.maxy, vol.bb.minz) , p2=(vol.bb.maxx, vol.bb.maxy, vol.bb.minz))  )
    lines.append( camvtk.Line(p1=(vol.bb.minx, vol.bb.maxy, vol.bb.maxz) , p2=(vol.bb.maxx, vol.bb.maxy, vol.bb.maxz))  )
    lines.append( camvtk.Line(p1=(vol.bb.minx, vol.bb.miny, vol.bb.maxz) , p2=(vol.bb.maxx, vol.bb.miny, vol.bb.maxz))  )
    
    lines.append( camvtk.Line(p1=(vol.bb.minx, vol.bb.miny, vol.bb.minz) , p2=(vol.bb.minx, vol.bb.miny, vol.bb.maxz))  )
    lines.append( camvtk.Line(p1=(vol.bb.maxx, vol.bb.miny, vol.bb.minz) , p2=(vol.bb.maxx, vol.bb.miny, vol.bb.maxz))  )
    
    lines.append( camvtk.Line(p1=(vol.bb.minx, vol.bb.maxy, vol.bb.minz) , p2=(vol.bb.minx, vol.bb.maxy, vol.bb.maxz))  )
    lines.append( camvtk.Line(p1=(vol.bb.maxx, vol.bb.maxy, vol.bb.minz) , p2=(vol.bb.maxx, vol.bb.maxy, vol.bb.maxz))  )
    
    lines.append( camvtk.Line(p1=(vol.bb.minx, vol.bb.miny, vol.bb.minz) , p2=(vol.bb.minx, vol.bb.maxy, vol.bb.minz))  )
    lines.append( camvtk.Line(p1=(vol.bb.maxx, vol.bb.miny, vol.bb.minz) , p2=(vol.bb.maxx, vol.bb.maxy, vol.bb.minz))  )
    
    lines.append( camvtk.Line(p1=(vol.bb.minx, vol.bb.miny, vol.bb.maxz) , p2=(vol.bb.minx, vol.bb.maxy, vol.bb.maxz))  )
    lines.append( camvtk.Line(p1=(vol.bb.maxx, vol.bb.miny, vol.bb.maxz) , p2=(vol.bb.maxx, vol.bb.maxy, vol.bb.maxz))  )    
    
    for l in lines:
        myscreen.addActor(l)

def main(filename="frame/f.png",yc=6, n=0):        
    f=ocl.Ocode()
    f.set_depth(9)
    f.set_scale(5)
    
    myscreen = camvtk.VTKScreen()   
    myscreen.camera.SetPosition(50, 22, 40)
    myscreen.camera.SetFocalPoint(0,0, 0)   
    myscreen.camera.Azimuth( n*0.5 )
    
    # box around octree 
    oct_cube = camvtk.Cube(center=(0,0,0), length=4*f.get_scale(), color=camvtk.white)
    oct_cube.SetWireframe()
    myscreen.addActor(oct_cube)
    
    # screenshot writer
    w2if = vtk.vtkWindowToImageFilter()
    w2if.SetInput(myscreen.renWin)
    lwr = vtk.vtkPNGWriter()
    lwr.SetInput( w2if.GetOutput() )
    arrowcenter=(1,2,0)
    xar = camvtk.Arrow(color=camvtk.red, center=arrowcenter, rotXYZ=(0,0,0))
    myscreen.addActor(xar)
    yar = camvtk.Arrow(color=camvtk.green, center=arrowcenter, rotXYZ=(0,0,90))
    myscreen.addActor(yar)
    zar = camvtk.Arrow(color=camvtk.blue, center=arrowcenter, rotXYZ=(0,-90,0))
    myscreen.addActor(zar) 
    
    """
    dl = myscreen.GetLights()
    print("original default light:")
    print(dl)
    print("nextitem()")
    l1 = dl.GetNextItem()
    print(" light:")
    print(l1)
    #print myscreen.GetLights()
    
    lights = vtk.vtkLightCollection()
    l = myscreen.MakeLight()
    l2 = myscreen.MakeLight()
    #myscreen.RemoveAllLights()
    l.SetAmbientColor(0.5, 0.5, 0.5)
    l.SetPosition(0,0,20)  
    l.SetConeAngle(360)  
    l2.SetPosition(0,0,-20)  
    l2.SetConeAngle(360)
    l2.SetIntensity(0.5) 
    myscreen.AddLight(l)
    myscreen.AddLight(l2)
    #myscreen.SetLightCollection(lights)
    llist = myscreen.GetLights()
    li = llist.GetNextItem()
    print(" new list of lights:")
    print(li)
    #for li in llist:
    #    print(li)
    print(" newly created light:")
    print(l)
    
    dl = myscreen.GetLights()
    print("NEW light:")
    print(dl)
    """
    
    t = ocl.LinOCT()
    t2 = ocl.LinOCT()
    t.init(0)
    t2.init(1)
    
    #drawTree2(myscreen, t, opacity=0.2)
    #myscreen.render()
    #myscreen.iren.Start() 
    #exit()
    
    print(" after init() t :", t.str())
    print(" after init() t2 :", t2.str())
    
    # sphere
    svol = ocl.SphereOCTVolume()
    svol.radius=3.2
    svol.center = ocl.Point(1,0,3)
    svol.calcBB()

    # cube
    cube1 = ocl.CubeOCTVolume()
    cube1.side=2.123
    cube1.center = ocl.Point(0,0,0)
    cube1.calcBB()
    
    #cylinder
    cylvol = ocl.CylinderOCTVolume()
    cylvol.p2 = ocl.Point(3,4,-5)
    cylvol.radius= 2
    cylvol.calcBB()
    
    # draw exact cylinder
    cp = 0.5*(cylvol.p1 + cylvol.p2)
    height = (cylvol.p2-cylvol.p1).norm()
    cylvolactor = camvtk.Cylinder(center=(cp.x, cp.y, cp.z-float(height)/2), radius = cylvol.radius, height=height, rotXYZ=(90,0,0))
    cylvolactor.SetWireframe()
    #myscreen.addActor(cylvolactor)

    
    c = ocl.CylCutter(2)
    c.length = 3
    print("cutter length=", c.length)
    p1 = ocl.Point(-1,-2,0)
    p2 = ocl.Point(1,2.0,0)
    g1vol = ocl.CylMoveOCTVolume(c, p1, p2)
   
    
    cyl1 = camvtk.Cylinder(center=(p1.x,p1.y,p1.z), radius=c.radius,
                            height=c.length,
                            rotXYZ=(90,0,0), color=camvtk.grey)
    cyl1.SetWireframe()
    myscreen.addActor(cyl1) 
    cyl2 = camvtk.Cylinder(center=(p2.x,p2.y,p2.z), radius=c.radius,
                            height=c.length,
                            rotXYZ=(90,0,0), color=camvtk.grey)
    cyl2.SetWireframe()
    myscreen.addActor(cyl2) 
    
    startp = camvtk.Sphere(center=(p1.x,p1.y,p1.z), radius=0.1, color=camvtk.green)
    myscreen.addActor(startp)
    
    endp = camvtk.Sphere(center=(p2.x,p2.y,p2.z), radius=0.1, color=camvtk.red)
    myscreen.addActor(endp)
    
    
       
    t.build( g1vol )
    t2.build( cube1)
    print("calling diff()...",)
    dt = t2.operation(1,t)
    print("done.")
    
    # set Cylinde bounding-box
    """
    cylvol.bb.maxx = 1.23
    cylvol.bb.minx = -0.2
    cylvol.bb.maxy = 1.23
    cylvol.bb.miny = -0.2
    cylvol.bb.maxz = 1.23
    cylvol.bb.minz = -0.2
    """
    drawBB( myscreen, g1vol)
    
    
    #print cylvol.bb.maxx
    
    #print "t2 build()" 
    #t2.build(cube1)
    #print " t2 after build() ", t2.size()
    #t2.condense()
    #print " t2 after condense() ", t2.size()   
    
    # original trees
    drawTree2(myscreen,t,opacity=1, color=camvtk.green)
    drawTree2(myscreen,t2,opacity=1, color=camvtk.cyan)
    drawTree2(myscreen,dt,opacity=1, color=camvtk.cyan, offset=(5,0,0))
    
    """
    for n in range(0,30):
        tp = ocl.Point(2.5,2.5,2-n*0.3)
        tpc = camvtk.black
        if (cylvol.isInside(tp)):
            tpc = camvtk.red
        else:
            tpc = camvtk.cyan
        tp_sphere = camvtk.Sphere(center=(tp.x,tp.y,tp.z), radius=0.1, color= tpc)
        myscreen.addActor(tp_sphere)
    """
    
    #drawTree(myscreen,t2,opacity=1, color=camvtk.red)

    #print " diff12()...",
    #t3 = t2.operation(1,t)
    #print "done."


    #print " diff21()...",
    #t4 = t2.operation(2,t)
    #print "done."


    #print " intersection()...",
    #t5 = t2.operation(3,t)
    #print "done."
    
    #print " sum()...",
    #t6 = t2.operation(4,t)
    #print "done."
    
    #print "  difference 1-2  t3 (blue) =", t3.size()
    #print " difference 2-1  t4 (yellow)=", t4.size()
    #print "     intersection t5 (pink) =", t5.size()
    #print "            union t6 (grey) =", t6.size()

    #drawTree(myscreen,t3,opacity=1, color=camvtk.blue, offset=(0,15,0))
    #drawTree(myscreen,t4,opacity=1, color=camvtk.yellow,offset=(0,-15,0))
    #drawTree(myscreen,t5,opacity=1, color=camvtk.pink,offset=(-15,0,0))
    #drawTree(myscreen,t6,opacity=1, color=camvtk.grey,offset=(-15,-15,0))
    
    
    title = camvtk.Text()
    title.SetPos( (myscreen.width-350, myscreen.height-30) )
    title.SetText("OpenCAMLib " + datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S"))
    myscreen.addActor(title)
    
    #st2 = camvtk.Text()
    #ytext = "Linear OCTree set operations: difference, intersection, union"
    #st2.SetText(ytext)
    #st2.SetPos( (50, myscreen.height-30) )   
    #myscreen.addActor( st2)
    
    #st3 = camvtk.Text()
    #text = "Original OCTrees\n  Ball:%d nodes\n  Cube: %d nodes" % ( t.size(), t2.size() )
    #st3.SetText(text)
    #st3.SetPos( (50, 200) )   
    #myscreen.addActor( st3)
    
    #st4 = camvtk.Text()
    #un = " Union (grey): %d nodes\n" % (t6.size())
    #int = " Intersection (pink): %d nodes\n"  % (t5.size())
    #diff1 = " difference Cube-Ball (blue): %d nodes\n"  % (t3.size())
    #diff2 = " difference Ball-Cube (yellow): %d nodes\n"  % (t4.size())
    #text= un+int+diff1+diff2
    #st4.SetText(text)
    #st4.SetPos( (50, 100) )   
    #myscreen.addActor( st4)
    
    print(" render()...",)
    myscreen.render()
    print("done.")
    lwr.SetFileName(filename)
    time.sleep(0.2)
    #lwr.Write()
    
    
    myscreen.iren.Start() 

if __name__ == "__main__":
    #Nsteps = 720
    #ystart = 6
    #ystop = -6
    #ystep = float(ystop-ystart)/(Nsteps-1)
    main()
        
        
        




    #fiangle = fiangle + 2
