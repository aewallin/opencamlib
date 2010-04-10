import ocl
import camvtk
import time
import datetime
import vtk



def drawTree(myscreen,t,color=camvtk.red,opacity=0.2, offset=(0,0,0)):
    nodes = t.get_nodes()
    black=0
    for n in nodes:
        cen = n.point()
        #print "cen=",cen.str()
        scale = n.get_scale()
        #print "col=", n.color
        
        if n.color == 0:
            #print "found white node!"
            #color = camvtk.red
            cube = camvtk.Cube(center=(cen.x+offset[0], cen.y+offset[1], cen.z+offset[2]), length= scale, color=color)
            cube.SetOpacity(opacity)
            #cube.SetPhong()
            cube.SetGouraud()
            myscreen.addActor( cube )
            black = black+1
        if n.color == 1:
            #print "found white node!"6
            color = camvtk.blue
            cube = camvtk.Cube(center=(cen.x, cen.y, cen.z), length= scale, color=color)
            cube.SetOpacity(opacity)
            myscreen.addActor( cube )
            #black = black+1
    #print black," black nodes"
    """
    for m in xrange(0,9):
        cen = n.corner(m)
        sph = camvtk.Sphere( center=(cen.x, cen.y, cen.z), radius=0.5, color=camvtk.green)
        myscreen.addActor(sph)
    """
        #myscreen.render()
        #raw_input("Press Enter to terminate")

def main(filename="frame/f.png",yc=6, n=0):        
    f=ocl.Ocode()
    f.set_depth(7)
    
    myscreen = camvtk.VTKScreen()   
    myscreen.camera.SetPosition(50, 22, 40)
    myscreen.camera.SetFocalPoint(0,0, 0)   
    myscreen.camera.Azimuth( n*0.5 )
    
    # screenshot writer
    w2if = vtk.vtkWindowToImageFilter()
    w2if.SetInput(myscreen.renWin)
    lwr = vtk.vtkPNGWriter()
    lwr.SetInput( w2if.GetOutput() )
    
    xar = camvtk.Arrow(color=camvtk.red, center=(10,20,0), rotXYZ=(0,0,0))
    myscreen.addActor(xar)
    yar = camvtk.Arrow(color=camvtk.green, center=(10,20,0), rotXYZ=(0,0,90))
    myscreen.addActor(yar)
    zar = camvtk.Arrow(color=camvtk.blue, center=(10,20,0), rotXYZ=(0,-90,0))
    myscreen.addActor(zar) 
    
    """
    dl = myscreen.GetLights()
    print "original default light:"
    print dl
    print "nextitem()"
    l1 = dl.GetNextItem()
    print " light:"
    print l1
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
    print " new list of lights:"
    print li
    #for li in llist:
    #    print li
    print " newly created light:"
    print l
    
    dl = myscreen.GetLights()
    print "NEW light:"
    print dl
    """
    
    t = ocl.LinOCT()
    #t2 = ocl.LinOCT()
    t.init(3)
    #t2.init(3)

    print " after init() t :", t.str()
    #print " after init() t2 :", t2.str()
    
    # sphere
    svol = ocl.SphereOCTVolume()
    svol.radius=3.2
    svol.center = ocl.Point(1,0,3)

    # cube
    cube1 = ocl.CubeOCTVolume()
    cube1.side=6
    cube1.center = ocl.Point(0,0,0)
    
    #cylinder
    
    cylvol = ocl.CylinderOCTVolume()
    cylvol.p2 = ocl.Point(0,0,4)
    cylvol.radius= 4
    print "t build()"    
    t.build(cylvol)
    
    print " t after build() ", t.size()
    
    #print "t2 build()" 
    #t2.build(cube1)
    #print " t2 after build() ", t2.size()
    #t2.condense()
    #print " t2 after condense() ", t2.size()   
    
    # original trees
    drawTree(myscreen,t,opacity=1, color=camvtk.green)
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
    
    
    myscreen.render()
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
