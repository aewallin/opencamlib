import vtk
import time
import datetime
import ocl as cam

class VTKScreen():
    """
    a vtk render window for displaying geometry
    """
    def __init__(self, width=1280, height=720):
        self.width=width
        self.height=height

        self.ren = vtk.vtkRenderer()
        self.renWin = vtk.vtkRenderWindow()
        self.renWin.AddRenderer(self.ren)
        self.renWin.SetSize(self.width,self.height)
        
        self.iren = vtk.vtkRenderWindowInteractor()
        self.iren.SetRenderWindow(self.renWin)
        interactorstyle = self.iren.GetInteractorStyle()
        interactorstyle.SetCurrentStyleToTrackballCamera()     
           
        self.camera = vtk.vtkCamera()
        self.camera.SetClippingRange(1, 100)
        self.camera.SetFocalPoint(0, 0, 0)
        self.camera.SetPosition(0, 35, 5)
        self.camera.SetViewAngle(30)
        self.camera.SetViewUp(0, 0, 1)
        self.ren.SetActiveCamera(self.camera)


        self.iren.Initialize()
                
    def addActor(self, actor):
        self.ren.AddActor(actor)

    def render(self):
        self.renWin.Render()




class Cone(vtk.vtkActor):
    def __init__(self, resolution=60, center=(-2,0,0), color=(1,1,0) ):
        self.src = vtk.vtkConeSource()
        self.src.SetResolution(resolution)
        self.src.SetCenter(center)

        self.mapper = vtk.vtkPolyDataMapper()
        self.mapper.SetInput(self.src.GetOutput())
        self.SetMapper(self.mapper)
        self.SetColor(color)

    def SetColor(self, color):
        self.GetProperty().SetColor(color)


class Sphere(vtk.vtkActor):
    def __init__(self, radius=1, thetaresolution=5, phiresolution=5, center=(0,2,0),
                color=(1,0,0)):
        self.src = vtk.vtkSphereSource()
        self.src.SetRadius(radius)
        self.src.SetCenter(center)
        self.src.SetThetaResolution(thetaresolution)
        self.src.SetPhiResolution(phiresolution)

        self.mapper = vtk.vtkPolyDataMapper()
        self.mapper.SetInput(self.src.GetOutput())
        self.SetMapper(self.mapper)
        self.SetColor(color)

    def SetColor(self, color):
        self.GetProperty().SetColor(color)

class Cube(vtk.vtkActor):
    def __init__(self,center=(2,2,0) , color=(0,1,0) ):
        self.src = vtk.vtkCubeSource()
        self.src.SetCenter(center)

        self.mapper = vtk.vtkPolyDataMapper()
        self.mapper.SetInput(self.src.GetOutput())
        self.SetMapper(self.mapper)
        self.SetColor(color)

    def SetColor(self, color):
        self.GetProperty().SetColor(color)



class Cylinder(vtk.vtkActor):
    def __init__(self,center=(0,-2,0) , radius=0.5, height=2, color=(0,1,1),
                    resolution=50 ):
        self.src = vtk.vtkCylinderSource()
        self.src.SetCenter(center)
        self.src.SetHeight(height)
        self.src.SetRadius(radius)
        self.src.SetResolution(resolution)
        # SetResolution
        # SetCapping(int)
        # CappingOn() CappingOff()

        self.mapper = vtk.vtkPolyDataMapper()
        self.mapper.SetInput(self.src.GetOutput())
        self.SetMapper(self.mapper)
        self.SetColor(color)

    def SetColor(self, color):
        self.GetProperty().SetColor(color)


class Line(vtk.vtkActor):
    # Get/SetResolution(int)
    def __init__(self,p1=(0,0,0) , p2=(1,1,1), color=(0,1,1) ):   
        self.src = vtk.vtkLineSource()
        self.src.SetPoint1(p1)
        self.src.SetPoint2(p2)

        self.mapper = vtk.vtkPolyDataMapper()
        self.mapper.SetInput(self.src.GetOutput())
        self.SetMapper(self.mapper)
        self.SetColor(color)

    def SetColor(self, color):
        self.GetProperty().SetColor(color)


class Point(vtk.vtkActor):
    """ vtkPointSource is a source object that creates a user-specified number of points within a specified radius about a specified center point. By default location of the points is random within the sphere. It is also possible to generate random points only on the surface of the sphere.
    """
    # Get/SetResolution(int) (?)
    def __init__(self, center=(0,0,0), color=(1,2,3) ):   
        self.src = vtk.vtkPointSource()
        self.src.SetCenter(center)
        self.src.SetRadius(0)
        self.src.SetNumberOfPoints(1)

        self.mapper = vtk.vtkPolyDataMapper()
        self.mapper.SetInput(self.src.GetOutput())
        self.SetMapper(self.mapper)
        self.SetColor(color)

    def SetColor(self, color):
        self.GetProperty().SetColor(color)




class Arrow(vtk.vtkActor):
    def __init__(self, center=(0,0,0), color=(0,0,1) ):
        self.src = vtk.vtkArrowSource()
        #self.src.SetCenter(center)

        self.mapper = vtk.vtkPolyDataMapper()
        self.mapper.SetInput(self.src.GetOutput())
        self.SetMapper(self.mapper)
        self.SetColor(color)

    def SetColor(self, color):
        self.GetProperty().SetColor(color)


class Text(vtk.vtkTextActor):
    def __init__(self, text="text",size=18,color=(1,1,1),pos=(100,100)):
        self.SetText(text)
        self.properties=self.GetTextProperty()
        self.properties.SetFontFamilyToArial()
        self.properties.SetFontSize(size)
        
        self.SetColor(color)
        self.SetPos(pos)
    
    def SetColor(self,color):
        self.properties.SetColor(color)
    
    def SetPos(self, pos):
        self.SetDisplayPosition(pos[0], pos[1])

    def SetText(self, text):
        self.SetInput(text)

class Axes(vtk.vtkActor):
    def __init__(self, center=(0,0,0), color=(0,0,1) ):
        self.src = vtk.vtkAxes()
        #self.src.SetCenter(center)

        self.mapper = vtk.vtkPolyDataMapper()
        self.mapper.SetInput(self.src.GetOutput())
        self.SetMapper(self.mapper)

        self.SetColor(color)
        self.SetOrigin(center)
        # SetScaleFactor(double)
        # GetOrigin
     

    def SetColor(self, color):
        self.GetProperty().SetColor(color)
    
    def SetOrigin(self, center=(0,0,0)):
        self.src.SetOrigin(center[0], center[1], center[2])

class STLSurf(vtk.vtkActor):
    def __init__(self, filename="", color=(1,1,1) ):
        self.src = vtk.vtkSTLReader()
        self.src.SetFileName(filename)
        self.src.Update()

        self.mapper = vtk.vtkPolyDataMapper()
        self.mapper.SetInput(self.src.GetOutput())
        self.SetMapper(self.mapper)

        self.SetColor(color)
        # SetScaleFactor(double)
        # GetOrigin
     
    def SetWireframe(self):
		self.GetProperty().SetRepresentationToWireframe()

    def SetColor(self, color):
        self.GetProperty().SetColor(color)    


class Plane(vtk.vtkActor):
    def __init__(self, center=(0,0,0), color=(0,0,1) ):
        self.src = vtk.vtkPlaneSource()
        #self.src.SetCenter(center)

        self.mapper = vtk.vtkPolyDataMapper()
        self.mapper.SetInput(self.src.GetOutput())
        self.SetMapper(self.mapper)

        self.SetColor(color)
        self.SetOrigin(center)
        # SetScaleFactor(double)
        # GetOrigin
     

    def SetColor(self, color):
        self.GetProperty().SetColor(color)    


# TODO:
# vtkArcSource
# vtkDiskSource
# vtkEarthSource (?)
# vtkFrustumSource
# vtkOutlineSource
# vtkParametricFunctionSource
# PlatonicSolid
# ProgrammableSource (?)
# PSphereSource
# RegularPolygon

#----------------------------------------------------------------

#---- misc helper functions
def vtkPolyData2CamalaSTL(vtkPolyData,camalaSTL):
	""" read vtkPolyData and add each triangle to a camala.STLSurf """
	for cellId in range(0,vtkPolyData.GetNumberOfCells()):
		cell = vtkPolyData.GetCell(cellId)
		points = cell.GetPoints()
		plist = []
		for pointId in range(0,points.GetNumberOfPoints()):
			vertex = points.GetPoint(pointId)
			p = cam.Point(vertex[0],vertex[1],vertex[2])
			plist.append(p)
		t = cam.Triangle(plist[0],plist[1],plist[2])
		camalaSTL.addTriangle(t)
