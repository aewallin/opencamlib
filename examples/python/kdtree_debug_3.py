import ocl as cam
import camvtk
import time
import vtk
import math
import datetime

    

def kdtreesearch(myscreen, tlist, s, cutter, cl, depth):
    #print "surface=", s.str()
    #print "cutter=", cutter.str()
    #print "cl=", cl.str()
    myscreen.render()
    #raw_input("Press Enter to terminate")
    #time.sleep(1)
    if (depth==1): # stop jumping and return all triangles
        tris = s.get_kd_triangles()
        for t in tris:
            tlist.append(t)
        return
    
    # jump high or low depending on search
    cut = s.get_kd_cut()
    print("cutvalues: ", cut)
    dim = cut[0]
    cval = cut[1]
    if dim == 0: # cut along xmax
        
        print(cval, " < ", cl.x - cutter.radius, " ??")
        if ( cval < ( cl.x - cutter.radius) ):
            myscreen.addActor( camvtk.Line( p1=(cval,100,0), p2=(cval,-100,0), color = camvtk.green ) )
            s.jump_kd_lo()
            trilist = s.get_kd_triangles()
            drawtriangles(myscreen, trilist, camvtk.blue)
            s.jump_kd_up()
            print("xmax: jump ONLY hi" )
            s.jump_kd_hi()
            print("search hi at level=", s.get_kd_level())
            kdtreesearch(myscreen, tlist, s, cutter, cl, depth-1)
            print("len tlist=", len(tlist), " now level=", s.get_kd_level())
        else:
            #print "xmax: jump both hi and lo"
            s.jump_kd_hi()
            #print "search hi at level=", s.get_kd_level()
            kdtreesearch(myscreen, tlist, s, cutter, cl, depth-1)
            #print "len tlist=", len(tlist), " now level=", s.get_kd_level()
            s.jump_kd_up()
            s.jump_kd_lo()
            #print "search lo at level=", s.get_kd_level()
            kdtreesearch(myscreen, tlist, s, cutter, cl, depth-1)
            #print "len tlist=", len(tlist), " now level=", s.get_kd_level()
    if dim == 1:
        
        print(cval, " > ", cl.x + cutter.radius, " ??")
        if ( cval > ( cl.x + cutter.radius) ):
            myscreen.addActor( camvtk.Line( p1=(cval,100,0), p2=(cval,-100,0), color = camvtk.lgreen ) )
            s.jump_kd_hi()
            trilist = s.get_kd_triangles()
            drawtriangles(myscreen, trilist, camvtk.blue)
            s.jump_kd_up()
            print("xmin: jump only lo")
            s.jump_kd_lo()
            kdtreesearch(myscreen, tlist, s, cutter, cl, depth-1)
        else:
            #print "xmin: jump both hi and lo"
            s.jump_kd_lo()
            kdtreesearch(tlist, s, cutter, cl, depth-1)
            s.jump_kd_up()
            s.jump_kd_hi()
            kdtreesearch(myscreen, tlist, s, cutter, cl, depth-1)
    if dim == 2:
        print(cval, " < ", cl.y - cutter.radius, " ??")
        if ( cval < ( cl.y - cutter.radius) ):
            myscreen.addActor( camvtk.Line( p1=(100,cval,0), p2=(-100,cval,0), color = camvtk.red ) )
            s.jump_kd_lo()
            trilist = s.get_kd_triangles()
            drawtriangles(myscreen, trilist, camvtk.yellow)
            s.jump_kd_up()
            
            s.jump_kd_hi()
            kdtreesearch(myscreen, tlist, s, cutter, cl, depth-1)
        else:
            #print "ymax: jump both hi and lo"
            s.jump_kd_lo()
            kdtreesearch(myscreen, tlist, s, cutter, cl, depth-1)
            s.jump_kd_up()
            s.jump_kd_hi()
            kdtreesearch(myscreen, tlist, s, cutter, cl, depth-1)
    if dim == 3: # cut along ymin
        
        print(cval, " > ", cl.y + cutter.radius, " ??")
        if ( cval > ( cl.y + cutter.radius) ):
            myscreen.addActor( camvtk.Line( p1=(100,cval,0), p2=(-100,cval,0), color = camvtk.pink ) )
            
            s.jump_kd_hi()
            trilist = s.get_kd_triangles()
            drawtriangles(myscreen, trilist, camvtk.yellow)
            s.jump_kd_up()
            
            print("ymin: jump ONLY lo")
            s.jump_kd_lo()
            kdtreesearch(myscreen, tlist, s, cutter, cl, depth-1)
        else:
            #print "ymin: jump both hi and lo"
            s.jump_kd_hi()
            #print "search hi at level=", s.get_kd_level()
            kdtreesearch(myscreen, tlist, s, cutter, cl, depth-1)
            #print "len tlist=", len(tlist), " now level=", s.get_kd_level()
            s.jump_kd_up()
            s.jump_kd_lo()
            #print "search lo at level=", s.get_kd_level()
            kdtreesearch(myscreen, tlist, s, cutter, cl, depth-1)
        
    return

def drawtriangles(myscreen, trilist, color):
    cpp = camvtk.STLSurf(triangleList=trilist)
    cpp.SetColor(color)
    cpp.SetWireframe()
    myscreen.addActor(cpp)

def drawcuts(myscreen, s):
    cut = s.get_kd_cut()
    if ( cut[0] < 2 ):
        print("x cut ",)
        if ( cut[0] == 0):
            print("max" )
            myscreen.addActor( camvtk.Line( p1=(cut[1],100,0), p2=(cut[1],-100,0), color = camvtk.green ) )
        else:
            print("min" )
            myscreen.addActor( camvtk.Line( p1=(cut[1],100,0), p2=(cut[1],-100,0), color = camvtk.lgreen ) )
    else:
        print("y cut ",)
        if ( cut[0] == 2):
            print("max" )
            myscreen.addActor( camvtk.Line( p1=(100,cut[1],0), p2=(-100,cut[1],0), color = camvtk.red ) )
        else:
            print("min")
            myscreen.addActor( camvtk.Line( p1=(100,cut[1],0), p2=(-100,cut[1],0), color = camvtk.pink ) )
                
    if (s.jump_kd_hi()):
        drawcuts(myscreen, s)
        s.jump_kd_up()
    if (s.jump_kd_lo()):
        drawcuts(myscreen, s)
        s.jump_kd_up()
    return
    
    

def main():
    myscreen = camvtk.VTKScreen()
    focal = cam.Point(50, 0, 0)
    r = 300
    theta = (float(45)/360)*2*math.pi
    fi=45
    
    campos = cam.Point( r*math.sin(theta)*math.cos(fi), r*math.sin(theta)*math.sin(fi), r*math.cos(theta) ) 
    myscreen.camera.SetPosition(campos.x, campos.y, campos.z)
    myscreen.camera.SetFocalPoint(focal.x,focal.y, focal.z)
    
    t = camvtk.Text()
    t.SetPos( (myscreen.width-450, myscreen.height-30) )

    
    myscreen.addActor( t)
    t2 = camvtk.Text()
    ytext = "kd-tree debug" #"Y: %3.3f" % (ycoord)
    t2.SetText(ytext)
    t2.SetPos( (50, myscreen.height-50) )   
    myscreen.addActor( t2)
        
    w2if = vtk.vtkWindowToImageFilter()
    w2if.SetInput(myscreen.renWin)
    lwr = vtk.vtkPNGWriter()
    lwr.SetInput( w2if.GetOutput() )
  
    epos = cam.Epos()
    epos.setS(0,1)

    t.SetText("OpenCAMLib 10.04-beta, " + datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S"))
    
    
    #ycoord = 1.1
    
    stl = camvtk.STLSurf(filename="../stl/carpet2.stl")
    #stl = camvtk.STLSurf(filename="demo2.stl")
    print("STL surface read")
    myscreen.addActor(stl)
    stl.SetWireframe()
    stl.SetColor((0.5,0.5,0.5))
    polydata = stl.src.GetOutput()
    s= cam.STLSurf()
    camvtk.vtkPolyData2OCLSTL(polydata, s)
    print("STLSurf with ", s.size(), " triangles")
    
    cutterDiameter=7
    cutter = cam.CylCutter(cutterDiameter)
    
    cl = cam.Point(31, 42, 3)
    
    cutactor = camvtk.Cylinder(center=(cl.x,cl.y,cl.z), 
                                   radius=cutterDiameter/2, 
                                   height=2, 
                                   rotXYZ=(90,0,0),
                                   color=camvtk.green)
    myscreen.addActor( cutactor )
    
    # sphere to see (0,0)
    myscreen.addActor( camvtk.Sphere( center=(0,0,0), radius=0.2, color = camvtk.yellow ) )
  
    s.build_kdtree()
    print("built kd-tree")
    s.jump_kd_reset()
    

    cpp_tlist = s.getTrianglesUnderCutter(cl, cutter)
    
    py_tlist = []
    depth = 6
    kdtreesearch(myscreen, py_tlist, s, cutter, cl, depth)
    
    print("len(cpp_list) after search=", len(cpp_tlist))
    print("len(py_list) after search=", len(py_tlist))
    
    cpp = camvtk.STLSurf(triangleList=cpp_tlist)
    cpp.SetColor(camvtk.lgreen)
    cpp.SetWireframe()
    myscreen.addActor(cpp)
    
    py = camvtk.STLSurf(triangleList=py_tlist)
    py.SetColor(camvtk.pink)
    py.SetWireframe()
    myscreen.addActor(py)
    
    
    
    #drawcuts(myscreen, s)
    
    myscreen.render()
    myscreen.iren.Start()
    time.sleep(2)
    exit()
    
    tlist = s.get_kd_triangles()
    
    print("got", len(tlist), " triangles")
    
    while (s.jump_kd_hi()):
        lotris = s.get_kd_triangles()
        s.jump_kd_up()
        cut = s.get_kd_cut()
        s.jump_kd_lo()
        hitris = s.get_kd_triangles()
        lev = s.get_kd_level()
        
        print("l=", lev, " hi=", len(hitris), " lo=", len(lotris), " cut=", cut)
        
        if ( cut[0] < 2 ):
            print("x cut ",)
            if ( cut[0] == 0):
                print("max" )
                myscreen.addActor( camvtk.Line( p1=(cut[1],100,0), p2=(cut[1],-100,0), color = camvtk.green ) )
            else:
                print("min" )
                myscreen.addActor( camvtk.Line( p1=(cut[1],100,0), p2=(cut[1],-100,0), color = camvtk.lgreen ) )
        else:
            print("y cut ",)
            if ( cut[0] == 2):
                print("max" )
                myscreen.addActor( camvtk.Line( p1=(100,cut[1],0), p2=(-100,cut[1],0), color = camvtk.red ) )
            else:
                print("min")
                myscreen.addActor( camvtk.Line( p1=(100,cut[1],0), p2=(-100,cut[1],0), color = camvtk.pink ) )
            
        
        slo = camvtk.STLSurf(triangleList=lotris)
        slo.SetColor(camvtk.pink)
        slo.SetWireframe()
        shi = camvtk.STLSurf(triangleList=hitris)
        shi.SetColor(camvtk.lgreen)
        shi.SetWireframe()
        myscreen.addActor(slo)
        myscreen.addActor(shi)
        myscreen.render()
        #myscreen.iren.Start()
        #raw_input("Press Enter to terminate")   
        time.sleep(1)
        myscreen.removeActor(slo)
        myscreen.removeActor(shi)
    

  
    print("done.")
    myscreen.render()
    #lwr.SetFileName(filename)
    
    #raw_input("Press Enter to terminate")         
    time.sleep(0.2)
    lwr.Write()
    myscreen.iren.Start()


if __name__ == "__main__":
    main()
    #raw_input("Press Enter to terminate")    

    
