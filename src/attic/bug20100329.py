import ocl
import camvtk
import time

if __name__ == "__main__":  
    
    myscreen = camvtk.VTKScreen()

    myscreen.camera.SetPosition(3, 60, 15)
    myscreen.camera.SetFocalPoint(1, 1, 10)
    
    cutter = ocl.CylCutter(3.0)
    path = ocl.Path()
    
    path.append(ocl.Line(ocl.Point(1.0, 1.4, 0), ocl.Point(2.0, 1.4, 0)))
    cl=ocl.Point(1.4,1.4,0)
    s=ocl.STLSurf("cone_on_side.stl")
    
    stl = camvtk.STLSurf("cone_on_side.stl")
    myscreen.addActor(stl)
    stl.SetWireframe()
    
    
    #dcf = ocl.PathDropCutterFinish(s)
    #dcf.setCutter(cutter)
    #dcf.setPath(path)
    #dcf.run()
    #plist = dcf.getCLPoints()
    pf = ocl.ParallelFinish()
    pf.initSTLSurf(s, 1)
    
    plist=[]
    zbase=-5
    #for n in range(0,3):
    #    plist.append( ocl.Point(1 + 0.01*n, 1.4, zbase) )
    plist.append( ocl.Point(1.02, 1.4, zbase) )
    
    clpoints=[]
    ccpoints=[]
    for p in plist:
        cc = ocl.CCPoint()
        trilist = pf.getTrianglesUnderCutter(p, cutter)
        for t in trilist:
            #cutter.dropCutter(p,cc,t)
            cutter.vertexDrop(p,cc,t)
            cutter.edgeDrop(p,cc,t)
            cutter.facetDrop(p,cc,t)
            stl2 = camvtk.STLSurf(filename=None, triangleList=[t], color=camvtk.red )
            stl2.SetSurface()
            stl2.SetOpacity(0.3)
            myscreen.addActor(stl2)
            myscreen.addActor( camvtk.Sphere(center=(p.x,p.y,p.z) , radius=0.1, color=camvtk.ccColor(cc) ) )
            if cc.type != ocl.CCType.NONE:
                myscreen.addActor( camvtk.Sphere(center=(cc.x,cc.y,cc.z) , radius=0.1, color=(0.2,0.3,0.4) ) )
            myscreen.render()
            if p.z > 10:
                print("cl-point is ", p.str())
                print("cc-point is ", cc.str())
                print("t=", t.str())
                tripoints = t.getPoints()
                #for pt in tripoints:
                #    print(pt.x, " , ", pt.y, " , ", pt.z)
                break
            myscreen.removeActor(stl2)
            #time.sleep(0.01)
            
        #cutter.dropCutterSTL(p, cc, s) 
        clpoints.append(p)
        ccpoints.append(cc)
    print("drop-cutter done")
    
    for (cl,cc) in zip(clpoints,ccpoints):
        myscreen.addActor( camvtk.Point(center=(cl.x,cl.y,cl.z) , color=camvtk.ccColor(cc) ) )
        #myscreen.addActor( camvtk.Point(center=(cc.x,cc.y,cc.z) , color=camvtk.yellow ) )
        print("adding point", cl.str())
        trilist = pf.getTrianglesUnderCutter(p, cutter)
        stl2 = camvtk.STLSurf(filename=None, triangleList=trilist, color=camvtk.red )
        stl2.SetSurface()
        #myscreen.addActor(stl2)
        
        trilist=[]
        myscreen.render()
        time.sleep(0.1)
        #myscreen.removeActor(stl2)
        
    #p = plist[2]
    #for t in s.tris:
    #    print(t.str())
        
    #cutter.vertexDrop(p)
    
    #print "number of cl-points=", len(plist)
    #print 'this point ', p.str()
    #print 'z should be 10.0, but it is about 12.0'
    
    myscreen.render()
    myscreen.iren.Start()
