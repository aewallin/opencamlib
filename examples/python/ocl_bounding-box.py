import ocl
import camvtk
import time
import vtk
import datetime
import math

def drawBB(myscreen, bb):
    lines=[]
    # x-direction lines, red color
    lines.append( camvtk.Line( p1=(bb.minpt.x, bb.minpt.y, bb.minpt.z), p2=(bb.maxpt.x, bb.minpt.y, bb.minpt.z), color=camvtk.red) )
    lines.append( camvtk.Line( p1=(bb.minpt.x, bb.maxpt.y, bb.minpt.z), p2=(bb.maxpt.x, bb.maxpt.y, bb.minpt.z), color=camvtk.red) )
    lines.append( camvtk.Line( p1=(bb.minpt.x, bb.minpt.y, bb.maxpt.z), p2=(bb.maxpt.x, bb.minpt.y, bb.maxpt.z), color=camvtk.red) )
    lines.append( camvtk.Line( p1=(bb.minpt.x, bb.maxpt.y, bb.maxpt.z), p2=(bb.maxpt.x, bb.maxpt.y, bb.maxpt.z), color=camvtk.red) )
    
    # y-direction lines, green color
    lines.append( camvtk.Line( p1=(bb.minpt.x, bb.minpt.y, bb.minpt.z), p2=(bb.minpt.x, bb.maxpt.y, bb.minpt.z), color=camvtk.green) )
    lines.append( camvtk.Line( p1=(bb.maxpt.x, bb.minpt.y, bb.minpt.z), p2=(bb.maxpt.x, bb.maxpt.y, bb.minpt.z), color=camvtk.green) )
    lines.append( camvtk.Line( p1=(bb.minpt.x, bb.minpt.y, bb.maxpt.z), p2=(bb.minpt.x, bb.maxpt.y, bb.maxpt.z), color=camvtk.green) )
    lines.append( camvtk.Line( p1=(bb.maxpt.x, bb.minpt.y, bb.maxpt.z), p2=(bb.maxpt.x, bb.maxpt.y, bb.maxpt.z), color=camvtk.green) )

    # z-direction lines, blue color
    lines.append( camvtk.Line( p1=(bb.minpt.x, bb.minpt.y, bb.minpt.z), p2=(bb.minpt.x, bb.minpt.y, bb.maxpt.z), color=camvtk.blue) )
    lines.append( camvtk.Line( p1=(bb.maxpt.x, bb.minpt.y, bb.minpt.z), p2=(bb.maxpt.x, bb.minpt.y, bb.maxpt.z), color=camvtk.blue) )
    lines.append( camvtk.Line( p1=(bb.minpt.x, bb.maxpt.y, bb.minpt.z), p2=(bb.minpt.x, bb.maxpt.y, bb.maxpt.z), color=camvtk.blue) )
    lines.append( camvtk.Line( p1=(bb.maxpt.x, bb.maxpt.y, bb.minpt.z), p2=(bb.maxpt.x, bb.maxpt.y, bb.maxpt.z), color=camvtk.blue) )
        
    for l in lines:
        myscreen.addActor(l)
        
if __name__ == "__main__":  
    print(ocl.version())
    
    myscreen = camvtk.VTKScreen()
    
    stl = camvtk.STLSurf("../stl/gnu_tux_mod.stl")
    #stl = camvtk.STLSurf("../stl/beet_mm.stl")
    #stl = camvtk.STLSurf("../stl/Blade.stl")
    myscreen.addActor(stl)
    stl.SetWireframe()
    stl.SetColor((0.5,0.5,0.5))
    
    polydata = stl.src.GetOutput()
    s = ocl.STLSurf()
    camvtk.vtkPolyData2OCLSTL(polydata, s)
    print("STL surface read ", s.size(), " triangles")
    
    minimum_point = s.bb.minpt
    maximum_point = s.bb.maxpt
    print("min point =", minimum_point)
    print("max point =", maximum_point)
    print(s.getBounds())
    
    # render the min and max points
    myscreen.addActor( camvtk.Sphere( center=(minimum_point.x, minimum_point.y, minimum_point.z), radius=0.1, color=camvtk.red) )
    myscreen.addActor( camvtk.Sphere( center=(maximum_point.x, maximum_point.y, maximum_point.z), radius=0.1, color=camvtk.green) )
    
    # render a bounding-box
    drawBB( myscreen, s.bb   )

    myscreen.camera.SetPosition(3, 23, 15)
    myscreen.camera.SetFocalPoint(4, 5, 0)    
    t = camvtk.Text()
    t.SetText("OpenCAMLib")
    t.SetPos( (myscreen.width-200, myscreen.height-30) )
    myscreen.addActor( t)
    
    myscreen.render()
    myscreen.iren.Start()
    raw_input("Press Enter to terminate") 
