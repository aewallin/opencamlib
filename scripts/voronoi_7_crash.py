import ocl
import camvtk
import time
import vtk
import datetime
import math
import random

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
        self.setVDText(vd)
        self.scale=scale
        
        myscreen.addActor(self.vdtext)
        
    def setVDText(self, vd):
        self.Ngen = len( vd.getGenerators() )-3
        self.vdtext_text = "VD with " + str(self.Ngen) + " generators."
        self.vdtext.SetText( self.vdtext_text )
        
    def setGenerators(self, vd):
        if len(self.gen_pts)>0:
            myscreen.removeActor( self.generators ) 
        #self.generators=[]
        self.gen_pts = []
        for p in vd.getGenerators():
            self.gen_pts.append(self.scale*p)
        self.generators= camvtk.PointCloud(pointlist=self.gen_pts) 
        self.generators.SetPoints()
        myscreen.addActor(self.generators)
        self.setVDText(vd)
        myscreen.render() 
    
    def setFar(self, vd):
        for p in vd.getFarVoronoiVertices():
            p=self.scale*p
            myscreen.addActor( camvtk.Sphere( center=(p.x,p.y,p.z), radius=4, color=camvtk.pink ) )
        myscreen.render() 
            
            
    def setVertices(self, vd):
        for p in self.verts:
            myscreen.removeActor(p)
        self.verts = []
        for p in vd.getVoronoiVertices():
            p=self.scale*p
            actor = camvtk.Sphere( center=(p.x,p.y,p.z), radius=0.000005, color=self.vertexColor )
            self.verts.append(actor)
            myscreen.addActor( actor )
        myscreen.render() 
        
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
        myscreen.addActor( self.edge_actor )
        myscreen.render() 
    
    def setEdges(self, vd):
        for e in self.edges:
            myscreen.removeActor(e)
        self.edges = []
        for e in vd.getEdgesGenerators():
            p1 = self.scale*e[0]  
            p2 = self.scale*e[1] 
            actor = camvtk.Line( p1=( p1.x,p1.y,p1.z), p2=(p2.x,p2.y,p2.z), color=self.edgeColor )
            myscreen.addActor(actor)
            self.edges.append(actor)
        myscreen.render() 
        
    def setAll(self, vd):
        self.setGenerators(vd)
        #self.setFar(vd)
        self.setVertices(vd)
        self.setEdges(vd)

def addVertexSlow(myscreen, vd, vod, p):        
    pass
    
if __name__ == "__main__":  
    print ocl.revision()
    myscreen = camvtk.VTKScreen()
    myscreen.camera.SetPosition(0.000000001, 0, 0.0131) # 1200 for far view, 300 for circle view
    myscreen.camera.SetFocalPoint(-3.96968e-04, -1.94809e-03, 0)
    # (-1.71738e-03, 3.64353e-03, 0)
    #172 (-1.71738e-06, 3.64353e-06, 0)
    myscreen.camera.SetClippingRange(-1,1)
    camvtk.drawOCLtext(myscreen)
    
    w2if = vtk.vtkWindowToImageFilter()
    w2if.SetInput(myscreen.renWin)
    lwr = vtk.vtkPNGWriter()
    lwr.SetInput( w2if.GetOutput() )
    #w2if.Modified()
    #lwr.SetFileName("tux1.png")
    
    scale=1000
    myscreen.render()
    random.seed(42)
    far = 0.000010
    vd = ocl.VoronoiDiagram(far,1200)
    
    vod = VD(myscreen,vd,scale)
    #vod.setAll(vd)
    drawFarCircle(myscreen, scale*vd.getFarRadius(), camvtk.orange)
    #plist=[ocl.Point(61,61)  ]
    #plist.append(ocl.Point(-20,-20))
    #plist.append(ocl.Point(0,0)) 
    
    Nmax = 171
    # far = 0.000010 crashes at n=192
    plist=[]
    for n in range(Nmax):
        x=-far/2+far*random.random()
        y=-far/2+far*random.random()
        plist.append( ocl.Point(x,y) )
    

    #print plist[169]
    #exit()
    n=1
    t_before = time.time() 
    delay = 4 # 0.533
    ren = [170,171]
    for p in plist:
        if n in ren:
            vod.setAll(vd)
            myscreen.render()
            time.sleep(delay)
        
        #GENERATOR
        gp=scale*p
        gen_actor = camvtk.Sphere( center=(gp.x,gp.y,gp.z), radius=0.0001, color=camvtk.yellow )
        if n in ren:
            myscreen.addActor(gen_actor)
            myscreen.render()
            time.sleep(delay)
            
        #CLOSEST FACE
        print "PYTHON: adding generator: ",n," at ",p
        clp = scale*vd.getClosestFaceGenerator(p)
        print " closest generator is ", clp
        cl_actor = camvtk.Sphere( center=(clp.x,clp.y,clp.z), radius=0.0001, color=camvtk.green )
        if n in ren:
            myscreen.addActor(cl_actor)
            myscreen.render()
            time.sleep(delay)
            
        #SEED
        sv = scale*vd.getSeedVertex(p)
        print " seed vertex is ",sv
        seed_actor = camvtk.Sphere( center=(sv.x,sv.y,sv.z), radius=0.0001, color=camvtk.pink )
        if n in ren:
            myscreen.addActor(seed_actor)
            myscreen.render()
            time.sleep(delay)
        
        #if n != 192:
        vd.addVertexSite( p )

        
        #w2if.Modified() 
        #lwr.SetFileName("frames/vd500_"+ ('%05d' % n)+".png")
        #lwr.Write()
        n=n+1
        if n in ren:
            myscreen.removeActor(cl_actor)
            myscreen.removeActor(gen_actor)
            myscreen.removeActor(seed_actor)
        
    t_after = time.time()
    calctime = t_after-t_before
    print " VD done in ", calctime," s, ", calctime/Nmax," s per generator"
        
        
    #vod.setAll(vd)
    #vod.setGenerators(vd)
    #time.sleep(1)
    #vod.setVertices(vd)
    #vod.setEdges(vd)

    #vd.addVertexSite( ocl.Point(0,-20) )
    #vod.setGenerators(vd)
    #time.sleep(1)
    #vod.setVertices(vd)
    #vod.setEdges(vd)
    
    #vd.addVertexSite( ocl.Point(20,20) )
    #drawDiagram( myscreen, vd )
    
    #dle = vd.getDelaunayEdges()
    #print " got ",len(dle)," Delaunay edges"
    #for e in dle:
    #    drawEdge(myscreen,e, camvtk.red)
    print "PYTHON All DONE."

    #camvtk.drawArrows(myscreen,center=(-0.5,-0.5,-0.5))
    
    myscreen.render()    
    myscreen.iren.Start()
