import ocl
import camvtk
import time
import vtk
import datetime
import math
import random
import numpy as np

def drawVertex(myscreen, p, vertexColor, rad=1):
    myscreen.addActor( camvtk.Sphere( center=(p.x,p.y,p.z), radius=rad, color=vertexColor ) )

def drawEdge(myscreen, e, edgeColor=camvtk.yellow):
    p1 = e[0]
    p2 = e[1]
    myscreen.addActor( camvtk.Line( p1=( p1.x,p1.y,p1.z), p2=(p2.x,p2.y,p2.z), color=edgeColor ) )

def drawFarCircle(myscreen, r, circleColor):
    myscreen.addActor( camvtk.Circle( center=(0,0,0), radius=r, color=circleColor ) )

def drawDiagram( myscreen, vd ):
    drawFarCircle(myscreen, vd.getFarRadius(), camvtk.pink)
    
    for v in vd.getGenerators():
        drawVertex(myscreen, v, camvtk.green, 2)
    for v in vd.getVoronoiVertices():
        drawVertex(myscreen, v, camvtk.red, 1)
    for v in vd.getFarVoronoiVertices():
        drawVertex(myscreen, v, camvtk.pink, 10)
    vde = vd.getVoronoiEdges()
    
    print " got ",len(vde)," Voronoi edges"
    for e in vde:
        drawEdge(myscreen,e, camvtk.cyan)

class VD:
    def __init__(self, myscreen, vd, scale=1):
        self.myscreen = myscreen
        self.gen_pts=[ocl.Point(0,0,0)]
        self.generators = camvtk.PointCloud(pointlist=self.gen_pts)
        self.verts=[]
        self.far=[]
        self.edges =[]
        self.generatorColor = camvtk.green
        self.vertexColor = camvtk.red
        self.edgeColor = camvtk.cyan
        self.vdtext  = camvtk.Text()
        self.vdtext.SetPos( (50, myscreen.height-50) )
        self.Ngen = 0
        self.vdtext_text = ""
        self.scale=scale
        self.setVDText(vd)
        
        
        myscreen.addActor(self.vdtext)
        
    def setVDText(self, vd):
        self.Ngen = len( vd.getGenerators() )-3
        self.vdtext_text = "VD with " + str(self.Ngen) + " generators. SCALE= " + str(vd.getFarRadius())
        self.vdtext.SetText( self.vdtext_text )
        self.vdtext.SetSize(32)
        
    def setGenerators(self, vd):
        if len(self.gen_pts)>0:
            self.myscreen.removeActor( self.generators ) 
        #self.generators=[]
        self.gen_pts = []
        for p in vd.getGenerators():
            self.gen_pts.append(self.scale*p)
        self.generators= camvtk.PointCloud(pointlist=self.gen_pts) 
        self.generators.SetPoints()
        self.myscreen.addActor(self.generators)
        self.setVDText(vd)
        self.myscreen.render() 
    
    def setFar(self, vd):
        for p in vd.getFarVoronoiVertices():
            p=self.scale*p
            self.myscreen.addActor( camvtk.Sphere( center=(p.x,p.y,p.z), radius=4, color=camvtk.pink ) )
        self.myscreen.render() 
            
            
    def setVertices(self, vd):
        for p in self.verts:
            self.myscreen.removeActor(p)
        self.verts = []
        for p in vd.getVoronoiVertices():
            p=self.scale*p
            actor = camvtk.Sphere( center=(p.x,p.y,p.z), radius=0.000005, color=self.vertexColor )
            self.verts.append(actor)
            self.myscreen.addActor( actor )
        self.myscreen.render() 
        
    def setEdgesPolydata(self, vd):
        self.edges = []
        self.edges = vd.getEdgesGenerators()
        self.epts = vtk.vtkPoints()
        nid = 0
        lines=vtk.vtkCellArray()
        for e in self.edges:
            p1 = self.scale*e[0]
            p2 = self.scale*e[1] 
            self.epts.InsertNextPoint( p1.x, p1.y, p1.z)
            self.epts.InsertNextPoint( p2.x, p2.y, p2.z)
            line = vtk.vtkLine()
            line.GetPointIds().SetId(0,nid)
            line.GetPointIds().SetId(1,nid+1)
            nid = nid+2
            lines.InsertNextCell(line)
        
        linePolyData = vtk.vtkPolyData()
        linePolyData.SetPoints(self.epts)
        linePolyData.SetLines(lines)
        
        mapper = vtk.vtkPolyDataMapper()
        mapper.SetInput(linePolyData)
        
        self.edge_actor = vtk.vtkActor()
        self.edge_actor.SetMapper(mapper)
        self.edge_actor.GetProperty().SetColor( camvtk.cyan )
        self.myscreen.addActor( self.edge_actor )
        self.myscreen.render() 
    
    def setEdges(self, vd):
        for e in self.edges:
            myscreen.removeActor(e)
        self.edges = []
        for e in vd.getEdgesGenerators():
            p1 = self.scale*e[0]  
            p2 = self.scale*e[1] 
            actor = camvtk.Line( p1=( p1.x,p1.y,p1.z), p2=(p2.x,p2.y,p2.z), color=self.edgeColor )
            self.myscreen.addActor(actor)
            self.edges.append(actor)
        self.myscreen.render() 
        
    def setAll(self, vd):
        self.setGenerators(vd)
        #self.setFar(vd)
        self.setVertices(vd)
        self.setEdges(vd)

def addVertexSlow(myscreen, vd, vod, p):        
    pass

def drawDiag(far, framenr):
    myscreen = camvtk.VTKScreen()
    myscreen.camera.SetFocalPoint(0, 0, 0)
    camvtk.drawOCLtext(myscreen)
    w2if = vtk.vtkWindowToImageFilter()
    w2if.SetInput(myscreen.renWin)
    lwr = vtk.vtkPNGWriter()
    lwr.SetInput( w2if.GetOutput() )
    scale=1000
    #far = 0.00001
    vd = ocl.VoronoiDiagram(far,1200)
    camPos = 0.2* (far/0.0001)
    myscreen.camera.SetPosition(camPos/10000, 0, camPos) 
    myscreen.camera.SetClippingRange(-2*camPos,2*camPos)
    random.seed(42)
    vod = VD(myscreen,vd,scale)
    drawFarCircle(myscreen, scale*vd.getFarRadius(), camvtk.orange)
    Nmax = 300
    plist=[]
    for n in range(Nmax):
        x=-far/2+far*random.random()
        y=-far/2+far*random.random()
        plist.append( ocl.Point(x,y) )
    n=1
    #ren = [300]
    for p in plist:
        print "PYTHON: adding generator: ",n," at ",p
        #if n in ren:
        vd.addVertexSite( p )
        n=n+1
        
    vod.setAll(vd)
    myscreen.render()
            
    w2if.Modified() 
    lwr.SetFileName("frames/vd_scale_"+ ('%05d' % framenr)+".png")
    lwr.Write()
        
    print "PYTHON All DONE."


    
    myscreen.render()    
    #myscreen.iren.Start()


if __name__ == "__main__":  
    print ocl.revision()

    maxf = 0.1
    minf = 0.0000001
    lmaxf = math.log(maxf)
    lminf = math.log(minf)
    Nframes = 10
    lrange = np.arange(lmaxf,lminf, -(lmaxf-lminf)/Nframes)
    print lrange
    fars = []
    for l in lrange:
        f = math.exp(l)
        fars.append(f)
    print fars
    #exit()
    #farvals = [0.1 , 0.01]
    n=1
    for f in fars:
        drawDiag(f,n)
        n=n+1
    
