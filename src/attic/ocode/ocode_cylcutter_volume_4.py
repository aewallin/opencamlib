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
        if (nmax>100):
            print("i=", i)
            print("div=", (float(nmax)/10))
            if ( (i % (float(nmax)/10))==0):
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
    ofs =ocl.Point(offset[0],offset[1],offset[2])
    for n in nodes:
        p1 = n.corner(0)+ofs # + + +
        p2 = n.corner(1)+ofs # - + +
        p3 = n.corner(2)+ofs # + - +
        p4 = n.corner(3)+ofs # + + -
        p5 = n.corner(4)+ofs # + - -
        p6 = n.corner(5)+ofs # - + -
        p7 = n.corner(6)+ofs # - - +
        p8 = n.corner(7)+ofs # - - -
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
        if (nmax>100):
            if ( (i % (nmax/10))==0):
                print(".",)
        i=i+1
            
        #tlist.append(ocl.Triangle(p1,p2,p4))
        #tlist.append(ocl.Triangle(p1,p3,p5))
        #tlist.append(ocl.Triangle(p2,p3,p7))
        #tlist.append(ocl.Triangle(p2,p7,p8))
        #tlist.append(ocl.Triangle(p3,p7,p8))
    print(".actor.",)
    surf = camvtk.STLSurf(triangleList=tlist)
    surf.SetColor(color)
    surf.SetOpacity(opacity)
    print(".add.",)
    myscreen.addActor(surf)
    print(".done.")
    
def drawCylCutter(myscreen, c, p):
    cyl = camvtk.Cylinder(center=(p.x,p.y,p.z), radius=c.radius,
                            height=c.length,
                            rotXYZ=(90,0,0), color=camvtk.grey)
    cyl.SetWireframe()
    myscreen.addActor(cyl) 



def main(filename="frame/f.png",yc=6, n=0):  
    print(ocl.revision())
          
    f=ocl.Ocode()
    f.set_depth(7) # depth and scale set here.
    f.set_scale(1)
    
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
    
    # X Y Z arrows
    arrowcenter=(1,2,0)
    xar = camvtk.Arrow(color=camvtk.red, center=arrowcenter, rotXYZ=(0,0,0))
    myscreen.addActor(xar)
    yar = camvtk.Arrow(color=camvtk.green, center=arrowcenter, rotXYZ=(0,0,90))
    myscreen.addActor(yar)
    zar = camvtk.Arrow(color=camvtk.blue, center=arrowcenter, rotXYZ=(0,-90,0))
    myscreen.addActor(zar) 
    
    
    t = ocl.LinOCT()
    t2 = ocl.LinOCT()
    t.init(5)
    t2.init(4)
        
    print(" after init() t :", t.str())
    print(" after init() t2 :", t2.str())
    
    c = ocl.CylCutter(1) # cutter
    c.length = 3
    print("cutter length=", c.length)
    p1 = ocl.Point(-0.2,-0.2,0.2) # start of move
    p2 = ocl.Point(1.5,1.5,-1) # end of move
    
    # volume of g1 move 
    g1vol = ocl.CylMoveOCTVolume(c, p1, p2)
    
    
    # sphere
    svol = ocl.SphereOCTVolume()
    svol.radius=1
    svol.center = ocl.Point(0,0,1)
    svol.calcBB()

    # cube
    cube1 = ocl.CubeOCTVolume()
    cube1.side=2.123
    cube1.center = ocl.Point(0,0,0)
    cube1.calcBB()
    
    #cylinder volume at start of move
    cylvol = ocl.CylinderOCTVolume()
    cylvol.p1 = ocl.Point(p1)
    cylvol.p2 = ocl.Point(p1)+ocl.Point(0,0,c.length)
    cylvol.radius= c.radius
    cylvol.calcBB()
    
    # draw exact cylinder
    cp = 0.5*(cylvol.p1 + cylvol.p2)
    height = (cylvol.p2-cylvol.p1).norm()
    cylvolactor = camvtk.Cylinder(center=(cp.x, cp.y, cp.z-float(height)/2), radius = cylvol.radius, height=height, rotXYZ=(90,0,0))
    cylvolactor.SetWireframe()
    myscreen.addActor(cylvolactor)

    # cylinder at start of move
    #drawCylCutter(myscreen, c, p1)
    # cylinder at end of move
    drawCylCutter(myscreen, c, p2)
    
    # green ball at start of move
    startp = camvtk.Sphere(center=(p1.x,p1.y,p1.z), radius=0.1, color=camvtk.green)
    myscreen.addActor(startp)
    # red ball at end of move
    endp = camvtk.Sphere(center=(p2.x,p2.y,p2.z), radius=0.1, color=camvtk.red)
    myscreen.addActor(endp)
    
    
    
    
    # build g1 tree
    t_before = time.time()
    t.build( g1vol )
    t_after = time.time()
    print("g1 build took ", t_after-t_before," s")
    
    # build cube
    t_before = time.time()
    t2.build( cube1 )
    t_after = time.time()
    print("cube build took ", t_after-t_before," s")
    
    #t.sort()
    #t2.sort()
    
    print("calling diff()...",)
    t_before = time.time()
    #dt = t2.operation(1,t)
    t2.diff(t)
    t_after = time.time()
    print("done.")
    print("diff took ", t_after-t_before," s")
    
    print("diff has ", t2.size()," nodes")
    
    # original trees
    print("drawing trees")
    drawTree2(myscreen,t,opacity=1, color=camvtk.green)
    drawTree2(myscreen,t2,opacity=0.2, color=camvtk.cyan)
    drawTree2(myscreen,t2,opacity=1, color=camvtk.cyan, offset=(5,0,0))
        
    # elliptical tube
    pmax = p1 + 1.5* (p2-p1)
    pmin = p1 - 0.5* (p2-p1)
    myscreen.addActor( camvtk.Sphere(center=(pmax.x,pmax.y,pmax.z), radius=0.1, color=camvtk.lgreen) )
    myscreen.addActor( camvtk.Sphere(center=(pmin.x,pmin.y,pmin.z), radius=0.1, color=camvtk.pink) )
    aaxis = pmin + ocl.Point(-0.353553, 0.353553, 0)
    baxis = pmin + ocl.Point(0.0243494, 0.0243494, 0.126617)
    myscreen.addActor( camvtk.Sphere(center=(aaxis.x,aaxis.y,aaxis.z), radius=0.1, color=camvtk.orange) )
    myscreen.addActor( camvtk.Sphere(center=(baxis.x,baxis.y,baxis.z), radius=0.1, color=camvtk.yellow) )
   
    title = camvtk.Text()
    title.SetPos( (myscreen.width-350, myscreen.height-30) )
    title.SetText("OpenCAMLib " + datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S"))
    myscreen.addActor(title)
        
    print(" render()...",)
    myscreen.render()
    print("done.")
    lwr.SetFileName(filename)
    time.sleep(0.2)
    #lwr.Write()
    
    
    myscreen.iren.Start() 

if __name__ == "__main__":

    main()
        
        
        



