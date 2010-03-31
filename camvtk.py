"""@camvtk docstring
This module provides classes for visualizing CAD/CAM algorithms using VTK.
"""

import vtk
import time
import datetime
import ocl as cam
import math

white = (1,1,1)
red= (1,0,0)
green= (0,1,0)
lgreen = ( float(170)/255,float(255)/255,float(170)/255)
blue= (0,0,1)
cyan=  (0,1,1)
yellow= (1,1,0)
black = (0,0,0)
pink = ( float(255)/255,float(192)/255,float(203)/255)
grey = ( float(127)/255,float(127)/255,float(127)/255)
orange = ( float(255)/255,float(165)/255,float(0)/255)


def ccColor(cc):
    """ this function returns a different color depending on the type of
        the CC-point. Useful for visualizing CL or CC points """
    if cc.type==cam.CCType.FACET:
        col = (1,0,1)
    elif cc.type == cam.CCType.VERTEX:
        col = yellow
    elif cc.type == cam.CCType.EDGE:
        col = cyan
    elif cc.type == cam.CCType.NONE:
        col = white 
    elif cc.type == cam.CCType.ERROR:
        col = (0,0.5,1)
    return col   
    

class VTKScreen():
    """
    a vtk render window for displaying geometry
    """
    def __init__(self, width=1280, height=720):
        """ create a screen """
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
        self.camera.SetClippingRange(0.01, 1000)
        self.camera.SetFocalPoint(0, 0, 0)
        self.camera.SetPosition(0, 35, 5)
        self.camera.SetViewAngle(30)
        self.camera.SetViewUp(0, 0, 1)
        self.ren.SetActiveCamera(self.camera)
        self.iren.Initialize()
        
        
    def setAmbient(self, r, g, b):
        """ set ambient color """
        self.ren.SetAmbient(r, g, b)
                    
    def addActor(self, actor):
        """ add an actor """
        self.ren.AddActor(actor)
    
    def removeActor(self, actor):
        """ remove an actor"""
        self.ren.RemoveActor(actor)

    def render(self):
        """ render scene"""
        self.renWin.Render()


class CamvtkActor(vtk.vtkActor):
    """ base class for actors"""
    def __init__(self):
        """ do nothing"""
        pass
        
    def SetColor(self, color):
        """ set color of actor"""
        self.GetProperty().SetColor(color)
    
    def SetOpacity(self, op=0.5):
        """ set opacity of actor, 0 is see-thru (invisible)"""
        self.GetProperty().SetOpacity(op)   
    
    def SetWireframe(self):
        """ set surface to wireframe"""
        self.GetProperty().SetRepresentationToWireframe()
        
    def SetSurface(self):
        """ set surface rendering on"""
        self.GetProperty().SetRepresentationToSurface() 
        
    def SetPoints(self):
        """ render only points"""
        self.GetProperty().SetRepresentationToPoints()
        
    def SetFlat(self):     
        """ set flat shading"""
        self.GetProperty().SetInterpolationToFlat()
    
    def SetGouraud(self):
        """ set gouraud shading"""
        self.GetProperty().SetInterpolationToGouraud()
    
    def SetPhong(self):
        """ set phong shading"""
        self.GetProperty().SetInterpolationToPhong()
    
    # possible TODOs
    # specular
    # diffuse
    # ambient
    

class Cone(CamvtkActor):
    """ a cone"""
    def __init__(self, resolution=60, center=(-2,0,0), color=(1,1,0) ):
        """ cone"""
        self.src = vtk.vtkConeSource()
        self.src.SetResolution(resolution)
        self.src.SetCenter(center)
        self.mapper = vtk.vtkPolyDataMapper()
        self.mapper.SetInput(self.src.GetOutput())
        self.SetMapper(self.mapper)
        self.SetColor(color)

class Sphere(CamvtkActor):
    """ a sphere"""
    def __init__(self, radius=1, resolution=20, center=(0,2,0),
                color=(1,0,0)):
        """ create sphere"""
        self.src = vtk.vtkSphereSource()
        self.src.SetRadius(radius)
        self.src.SetCenter(center)
        self.src.SetThetaResolution(resolution)
        self.src.SetPhiResolution(resolution)

        self.mapper = vtk.vtkPolyDataMapper()
        self.mapper.SetInput(self.src.GetOutput())
        self.SetMapper(self.mapper)
        self.SetColor(color)

class Cube(CamvtkActor):
    """ a cube"""
    def __init__(self,center=(2,2,0) , color=(0,1,0) ):
        """ create cube"""
        self.src = vtk.vtkCubeSource()
        self.src.SetCenter(center)

        self.mapper = vtk.vtkPolyDataMapper()
        self.mapper.SetInput(self.src.GetOutput())
        self.SetMapper(self.mapper)
        self.SetColor(color)

class Cylinder(CamvtkActor):
    """ cylinder """
    def __init__(self,center=(0,-2,0) , radius=0.5, height=2, color=(0,1,1),
                    rotXYZ=(0,0,0), resolution=50 ):
        """ cylinder """
        self.src = vtk.vtkCylinderSource()
        self.src.SetCenter(0,0,0)
        self.src.SetHeight(height)
        self.src.SetRadius(radius)
        self.src.SetResolution(resolution)
        # SetResolution
        # SetCapping(int)
        # CappingOn() CappingOff()
        
        # this transform rotates the cylinder so it is vertical
        # and then translates the lower tip to the center point
        transform = vtk.vtkTransform()
        transform.Translate(center[0], center[1], center[2]+height/2)
        transform.RotateX(rotXYZ[0])
        transformFilter=vtk.vtkTransformPolyDataFilter()
        transformFilter.SetTransform(transform)
        transformFilter.SetInputConnection(self.src.GetOutputPort())
        transformFilter.Update()

        
        self.mapper = vtk.vtkPolyDataMapper()
        #self.mapper.SetInput(self.src.GetOutput())
        self.mapper.SetInput( transformFilter.GetOutput() )
        self.SetMapper(self.mapper)
        self.SetColor(color)


class Line(CamvtkActor):
    """ line """
    def __init__(self,p1=(0,0,0) , p2=(1,1,1), color=(0,1,1) ):   
        """ line """
        self.src = vtk.vtkLineSource()
        self.src.SetPoint1(p1)
        self.src.SetPoint2(p2)
        self.mapper = vtk.vtkPolyDataMapper()
        self.mapper.SetInput(self.src.GetOutput())
        self.SetMapper(self.mapper)
        self.SetColor(color)


class Circle(CamvtkActor):
    """ circle"""
    def __init__(self,center=(0,0,0) , radius=1, color=(0,1,1), resolution=50 ):   
        """ create circle """
        lines =vtk.vtkCellArray()
        id = 0
        points = vtk.vtkPoints()
        for n in xrange(0,resolution):
            line = vtk.vtkLine()
            angle1 = (float(n)/(float(resolution)))*2*math.pi
            angle2 = (float(n+1)/(float(resolution)))*2*math.pi
            p1 = (center[0]+radius*math.cos(angle1), center[1]+radius*math.sin(angle1), center[2])
            p2 = (center[0]+radius*math.cos(angle2), center[1]+radius*math.sin(angle2), center[2])
            points.InsertNextPoint(p1)
            points.InsertNextPoint(p2)
            line.GetPointIds().SetId(0,id)
            id=id+1
            line.GetPointIds().SetId(1,id)
            id=id+1
            lines.InsertNextCell(line)
            

        self.pdata = vtk.vtkPolyData()
        self.pdata.SetPoints(points)
        self.pdata.SetLines(lines)
        
        self.mapper = vtk.vtkPolyDataMapper()
        self.mapper.SetInput(self.pdata)
        self.SetMapper(self.mapper)
        self.SetColor(color)
        
class Tube(CamvtkActor):
    """ a Tube is a line with thickness"""
    def __init__(self, p1=(0,0,0) , p2=(1,1,1), radius=0.2, color=(0,1,1) ):   
        """ tube"""
        points = vtk.vtkPoints()
        points.InsertNextPoint(p1)
        points.InsertNextPoint(p2)
        line = vtk.vtkLine()
        line.GetPointIds().SetId(0,0)
        line.GetPointIds().SetId(1,1)
        lines =vtk.vtkCellArray()
        lines.InsertNextCell(line)
        self.pdata = vtk.vtkPolyData()
        self.pdata.SetPoints(points)
        self.pdata.SetLines(lines)

        tubefilter=vtk.vtkTubeFilter()
        tubefilter.SetInput(self.pdata)
        tubefilter.SetRadius(radius)
        tubefilter.SetNumberOfSides(50)
        tubefilter.Update()
        
        self.mapper = vtk.vtkPolyDataMapper()
        self.mapper.SetInput(tubefilter.GetOutput())
        self.SetMapper(self.mapper)
        self.SetColor(color)


class Point(CamvtkActor):
    """ point"""
    def __init__(self, center=(0,0,0), color=(1,2,3) ):   
        """ create point """
        self.src = vtk.vtkPointSource()
        self.src.SetCenter(center)
        self.src.SetRadius(0)
        self.src.SetNumberOfPoints(1)

        self.mapper = vtk.vtkPolyDataMapper()
        self.mapper.SetInput(self.src.GetOutput())
        self.SetMapper(self.mapper)
        self.SetColor(color)

class Arrow(CamvtkActor):
    """ arrow """
    def __init__(self, center=(0,0,0), color=(0,0,1), rotXYZ=(0,0,0) ):
        """ arrow """
        self.src = vtk.vtkArrowSource()
        #self.src.SetCenter(center)
        
        transform = vtk.vtkTransform()
        transform.Translate(center[0], center[1], center[2])
        transform.RotateX(rotXYZ[0])
        transform.RotateY(rotXYZ[1])
        transform.RotateZ(rotXYZ[2])
        transformFilter=vtk.vtkTransformPolyDataFilter()
        transformFilter.SetTransform(transform)
        transformFilter.SetInputConnection(self.src.GetOutputPort())
        transformFilter.Update()
        

        self.mapper = vtk.vtkPolyDataMapper()
        self.mapper.SetInput( transformFilter.GetOutput() )
        self.SetMapper(self.mapper)
        self.SetColor(color)


class Text(vtk.vtkTextActor):
    """ 2D text, HUD-type"""
    def __init__(self, text="text",size=18,color=(1,1,1),pos=(100,100)):
        """create text"""
        self.SetText(text)
        self.properties=self.GetTextProperty()
        self.properties.SetFontFamilyToArial()
        self.properties.SetFontSize(size)
        
        self.SetColor(color)
        self.SetPos(pos)
    
    def SetColor(self,color):
        """ set color of text """
        self.properties.SetColor(color)
    
    def SetPos(self, pos):
        """ set position on screen """
        self.SetDisplayPosition(pos[0], pos[1])

    def SetText(self, text):
        """ set text to be displayed """
        self.SetInput(text)

class Text3D(vtk.vtkActor):
    """ 3D text rendered in the scene"""
    def __init__(self, color=(1,1,1), center=(0,0,0), text="hello", scale=1):
        """ create text """
        self.src = vtk.vtkVectorText()
        self.SetText(text)
        #self.SetCamera(camera)
        transform = vtk.vtkTransform()
        transform.Translate(center[0], center[1], center[2])
        transform.Scale(scale, scale, scale)
        
        #transform.RotateY(90)
        #transform2 = vtk.vtkTransform()
        
        #transform.Concatenate(transform2)
        #transformFilter=vtk.vtkTransformPolyDataFilter()
        #transformFilter.SetTransform(transform)
        #transformFilter.SetInputConnection(self.src.GetOutputPort())
        #transformFilter.Update()
        self.SetUserTransform(transform)
        
        self.mapper = vtk.vtkPolyDataMapper()
        self.mapper.SetInputConnection(self.src.GetOutputPort())
        self.SetMapper(self.mapper)
        
        self.SetColor(color)
        
        
    def SetText(self, text):
        """ set text to be displayed"""
        self.src.SetText(text)
        
    def SetColor(self,color):
        """ set color of text"""
        self.GetProperty().SetColor(color)        

class Axes(vtk.vtkActor):
    """ axes (x,y,z) """
    def __init__(self, center=(0,0,0), color=(0,0,1) ):
        """ create axes """
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

class Toroid(CamvtkActor):
    def __init__(self, r1=1, r2=0.25, center=(0,0,0), rotXYZ=(0,0,0), color=(1,0,0)):
        self.parfun = vtk.vtkParametricSuperToroid()
        self.parfun.SetRingRadius(r1)
        self.parfun.SetCrossSectionRadius(r2)
        self.parfun.SetN1(1)
        self.parfun.SetN2(1)
         
        self.src = vtk.vtkParametricFunctionSource()
        self.src.SetParametricFunction(self.parfun)
        
        transform = vtk.vtkTransform()
        transform.Translate(center[0], center[1], center[2])
        transform.RotateX(rotXYZ[0])
        transform.RotateY(rotXYZ[1])
        transform.RotateZ(rotXYZ[2])
        transformFilter=vtk.vtkTransformPolyDataFilter()
        transformFilter.SetTransform(transform)
        transformFilter.SetInputConnection(self.src.GetOutputPort())
        transformFilter.Update()
        
        self.mapper = vtk.vtkPolyDataMapper()
        self.mapper.SetInput(transformFilter.GetOutput())
        self.SetMapper(self.mapper)
        self.SetColor(color)    

class TrilistReader(vtk.vtkPolyDataAlgorithm):
    def __init__(self, triangleList):
        vtk.vtkPolyDataAlgorithm.__init__(self)
        self.FileName = None
        self.SetNumberOfInputPorts(0)
        self.SetNumberOfOutputPorts(1)
        
    def FillOutputPortInfornmation(self, port, info):
        if port == 0:
            info.Set( vtk.vtkDataObject.DATA_TYPE_NAME(), "vtkPolyData")
            return 1
        return 0
        
    def RequestData(self, request, inputVector, outputVector):
        outInfo = outputVector.GetInformationObject(0)
        output = outInfo.Get( vtk.vtkDataObject.DATA_OBJECT() )
        polydata = vtk.vtkPolyData()
        points = vtk.vtkPoints()
        points.InsertNextPoint(0,0,0)
        polydata.SetPoints(points)
        
        output.ShallowCopy(polydata)
        return 1

class STLSurf(CamvtkActor):
    def __init__(self, filename=None, triangleList=[], color=(1,1,1) ):
        self.src=[];
        
        if filename is None:
            #self.src= TrilistReader(triangleList)
            #self.src.Update()
            #self.src = vtk.vtkPolyData()
            points = vtk.vtkPoints()
            triangles = vtk.vtkCellArray()
            n=0
            for t in triangleList:
                triangle = vtk.vtkTriangle()
                for p in t.getPoints():
                    points.InsertNextPoint(p.x, p.y, p.z)
                triangle.GetPointIds().SetId(0,n)
                n=n+1
                triangle.GetPointIds().SetId(1,n)
                n=n+1
                triangle.GetPointIds().SetId(2,n)
                n=n+1
                triangles.InsertNextCell(triangle)
            polydata= vtk.vtkPolyData()
            polydata.SetPoints(points)
            polydata.SetPolys(triangles)
            polydata.Modified()
            polydata.Update()
            self.src=polydata
            self.mapper = vtk.vtkPolyDataMapper()
            self.mapper.SetInput(self.src)
            self.SetMapper(self.mapper)
        else:
            self.src = vtk.vtkSTLReader()
            self.src.SetFileName(filename)
            self.src.Update()
            self.mapper = vtk.vtkPolyDataMapper()
            self.mapper.SetInput(self.src.GetOutput())
            self.SetMapper(self.mapper)

        self.SetColor(color)
        # SetScaleFactor(double)
        # GetOrigin

class Plane(CamvtkActor):
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
def vtkPolyData2OCLSTL(vtkPolyData,oclSTL):
    """ read vtkPolyData and add each triangle to an ocl.STLSurf """
    for cellId in range(0,vtkPolyData.GetNumberOfCells()):
        cell = vtkPolyData.GetCell(cellId)
        points = cell.GetPoints()
        plist = []
        for pointId in range(0,points.GetNumberOfPoints()):
            vertex = points.GetPoint(pointId)
            p = cam.Point(vertex[0],vertex[1],vertex[2])
            plist.append(p)
        t = cam.Triangle(plist[0],plist[1],plist[2])
        oclSTL.addTriangle(t)
