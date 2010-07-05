import ocl
import camvtk
import time
import vtk
import datetime


def CLPointGrid(minx,dx,maxx,miny,dy,maxy,z):
    plist = []
    xvalues = [round(minx+n*dx,2) for n in xrange(int(round((maxx-minx)/dx))+1) ]
    yvalues = [round(miny+n*dy,2) for n in xrange(int(round((maxy-miny)/dy))+1) ]
    for y in yvalues:
        for x in xvalues:
            plist.append( ocl.Point(x,y,z) )
    return plist

if __name__ == "__main__":  
    print ocl.revision()
    myscreen = camvtk.VTKScreen()
    stl = camvtk.STLSurf("../stl/gnu_tux_mod.stl")
    print "STL surface read"
    myscreen.addActor(stl)
    stl.SetWireframe()
    stl.SetColor((0.5,0.5,0.5))
	
    polydata = stl.src.GetOutput()
    s= ocl.STLSurf()
    camvtk.vtkPolyData2OCLSTL(polydata, s)
    
    cutter = ocl.CylCutter(0.6)
    print cutter

    minx=-0.2
    dx=0.1
    maxx=10.2
    miny=-0.2
    dy=1
    maxy=12.2
    z=-0.2
    clpoints = CLPointGrid(minx,dx,maxx,miny,dy,maxy,z)
    nv=0 # vertex hits
    nn=0 
    ne=0 # edge hits
    nf=0 # facet hits
    for cl in clpoints:
        #cutter.dropCutter(cl,cc,t)
        #cutter.dropCutterSTL(cl,cc,s)
        for t in s.tris:
            cutter.vertexDrop(cl,t)
        #    cutter.edgeDrop(cl,cc,t)
        #    cutter.facetDrop(cl,cc,t)
        
        if cl.cc.type == ocl.CCType.FACET:
            nf+=1
            col = (0,1,1)
        elif cl.cc.type == ocl.CCType.VERTEX:
            nv+=1
            col = (0,1,0)
        elif cl.cc.type == ocl.CCType.EDGE:
            ne+=1
            col = (1,0,0)
        elif cl.cc.type == ocl.CCType.NONE:
            #print "type=NONE!"
            nn+=1
            col = (1,1,1)
        
        myscreen.addActor( camvtk.Point(center=(cl.x,cl.y,cl.z) , color=col) )    
        #myscreen.addActor( camvtk.Point(center=(cc.x,cc.y,cc.z), color=col) )
        #print cc.type
        
    print "none=",nn," vertex=",nv, " edge=",ne, " facet=",nf, " sum=", nn+nv+ne+nf
    print len(clpoints), " cl points evaluated"
    myscreen.camera.SetPosition(3, 23, 15)
    myscreen.camera.SetFocalPoint(4, 5, 0)
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
        lwr.SetFileName("frame"+ ('%03d' % n)+".png")
        w2if.Modified() 
        #lwr.Write()

    #myscreen.iren.Start()
    raw_input("Press Enter to terminate") 
