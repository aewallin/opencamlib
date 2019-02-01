import ocl
import camvtk
import time
import vtk
import datetime
import math
import random

def drawVertex(myscreen, p, vertexColor, rad=0.1):
    myscreen.addActor( camvtk.Sphere( center=(p.x,p.y,p.z), radius=rad, color=vertexColor ) )

def drawEdge(myscreen, e, edgeColor=camvtk.yellow):
    p1 = e[0]
    p2 = e[1]
    myscreen.addActor( camvtk.Line( p1=( p1.x,p1.y,p1.z), p2=(p2.x,p2.y,p2.z), color=edgeColor ) )

#def drawFarCircle(myscreen, r, circleColor):
#    myscreen.addActor( camvtk.Circle( center=(0,0,0), radius=r, color=circleColor ) )

def drawDiagram( myscreen, diag ):
    #drawFarCircle(myscreen, vd.getFarRadius(), camvtk.pink)
    for v in diag.getVertices():
        drawVertex(myscreen, v, camvtk.green)
    edges = diag.getEdges()
    for e in edges:
        drawEdge(myscreen,e, camvtk.cyan)

def writeFrame( w2if, lwr, n ):
    w2if.Modified() 
    lwr.SetFileName("frames/vd500_zoomout"+ ('%05d' % n)+".png")
    lwr.Write()
    
if __name__ == "__main__":  
    print(ocl.version())
    myscreen = camvtk.VTKScreen()
    
    
    camvtk.drawOCLtext(myscreen)
    
    #w2if = vtk.vtkWindowToImageFilter()
    #w2if.SetInput(myscreen.renWin)
    #lwr = vtk.vtkPNGWriter()
    #lwr.SetInput( w2if.GetOutput() )
    #w2if.Modified()
    #lwr.SetFileName("tux1.png")

# SURFACE
    
    #stl = camvtk.STLSurf("../stl/Cylinder_1.stl")
    #stl = camvtk.STLSurf("../stl/gnu_tux_mod.stl")
    stl = camvtk.STLSurf("../stl/demo.stl")
    myscreen.addActor(stl)
    stl.SetWireframe()
    stl.SetColor((0.5,0.5,0.5))
    
    polydata = stl.src.GetOutput()
    s = ocl.STLSurf()
    camvtk.vtkPolyData2OCLSTL(polydata, s)
    print("STL surface read,", s.size(), "triangles")
    
    
    far = 20
    # far = 0.000002 generator 52 face_count crash
    # far = 0.000010 crashes at n=192
    
    camPos = 2* far
    myscreen.camera.SetPosition(camPos/1000, camPos/1000, camPos) 
    myscreen.camera.SetClippingRange(-2*camPos,2*camPos)
    myscreen.camera.SetFocalPoint(0.051, 0, 0)
    
    cls = ocl.CutterLocationSurface(10)
    
    cutter = ocl.BallCutter(2,10)
    cls.setCutter(cutter)
    cls.setSampling(1)
    cls.setMinSampling(0.1)
    cls.setSTL(s)
    
    drawDiagram(myscreen, cls)
    #vd = ocl.VoronoiDiagram(far,1200)
    
    #vod = VD(myscreen,vd,scale)
    #vod.setAll(vd)
    #drawFarCircle(myscreen, scale*vd.getFarRadius(), camvtk.orange)
    

        
    print("PYTHON All DONE.")


    
    myscreen.render()    
    myscreen.iren.Start()
