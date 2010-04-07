import ocl
import camvtk

def printNodes(t):
    nodes = t.get_nodes()
    for n in nodes:
        print n.str(), " c=",n.color

def drawTree(myscreen,t,color=camvtk.red,opacity=0.2):
    nodes = t.get_nodes()
    black=0
    for n in nodes:
        cen = n.point()
        #print "cen=",cen.str()
        scale = n.get_scale()
        #print "col=", n.color
        
        if n.color == 0:
            #print "found white node!"
            color = camvtk.red
            cube = camvtk.Cube(center=(cen.x, cen.y, cen.z), length= scale, color=color)
            cube.SetOpacity(opacity)
            myscreen.addActor( cube )
            black = black+1
    print black," black nodes"
    """
    for m in xrange(0,9):
        cen = n.corner(m)
        sph = camvtk.Sphere( center=(cen.x, cen.y, cen.z), radius=0.5, color=camvtk.green)
        myscreen.addActor(sph)
    """
        #myscreen.render()
        #raw_input("Press Enter to terminate")
        
f=ocl.Ocode()
f.set_depth(2)


o2 = ocl.Ocode()

myscreen = camvtk.VTKScreen()
    
myscreen.camera.SetPosition(80, 52, 20)
myscreen.camera.SetFocalPoint(0,0, 0)
    
t = ocl.LinOCT()

print t.str()
t.append(o2)
printNodes(t)
print "t.size=", t.size()
svol = ocl.SphereOCTVolume()
svol.radius=5
svol.center = ocl.Point(0,0,0.0)

sphvol = camvtk.Sphere(center=(svol.center.x,svol.center.y,svol.center.z), radius=svol.radius)
sphvol.SetColor(camvtk.blue)
sphvol.SetWireframe()
myscreen.addActor(sphvol)

#print "build()"    
t.build(svol,4)
#t.expand_at(0)

print "t.size=", t.size()
#printNodes(t)


xar = camvtk.Arrow(color=camvtk.red, rotXYZ=(0,0,0))
myscreen.addActor(xar)
yar = camvtk.Arrow(color=camvtk.green, rotXYZ=(0,0,90))
myscreen.addActor(yar)
zar = camvtk.Arrow(color=camvtk.blue, rotXYZ=(0,-90,0))
myscreen.addActor(zar)    

drawTree(myscreen,t,opacity=1.0)
#printNodes(t)
myscreen.render()
myscreen.iren.Start() 
    

#t.expand_at(1)
#print "t.size=", t.size()
#printNodes(t)
