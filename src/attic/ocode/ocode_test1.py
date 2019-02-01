import ocl
import camvtk
import time

def printNodes(t):
    nodes = t.get_nodes()
    for n in nodes:
        print(n.str(), " c=",n.color)

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
            #color = camvtk.red
            cube = camvtk.Cube(center=(cen.x, cen.y, cen.z), length= scale, color=color)
            cube.SetOpacity(opacity)
            myscreen.addActor( cube )
            black = black+1
        if n.color == 1:
            #print "found white node!"6
            color = camvtk.blue
            cube = camvtk.Cube(center=(cen.x, cen.y, cen.z), length= scale, color=color)
            cube.SetOpacity(opacity)
            myscreen.addActor( cube )
            #black = black+1
    print(black," black nodes")
    """
    for m in range(0,9):
        cen = n.corner(m)
        sph = camvtk.Sphere( center=(cen.x, cen.y, cen.z), radius=0.5, color=camvtk.green)
        myscreen.addActor(sph)
    """
        #myscreen.render()
        #raw_input("Press Enter to terminate")
        
f=ocl.Ocode()
f.set_depth(4)


myscreen = camvtk.VTKScreen()   
myscreen.camera.SetPosition(80, 52, 20)
myscreen.camera.SetFocalPoint(0,0, 0)   

xar = camvtk.Arrow(color=camvtk.red, rotXYZ=(0,0,0))
myscreen.addActor(xar)
yar = camvtk.Arrow(color=camvtk.green, rotXYZ=(0,0,90))
myscreen.addActor(yar)
zar = camvtk.Arrow(color=camvtk.blue, rotXYZ=(0,-90,0))
myscreen.addActor(zar) 


t = ocl.LinOCT()
print " created: ",t.str()
#t.append(o2)
t.init(2)
print " after init():", t.str()
#printNodes(t)
print "t.size=", t.size()
svol = ocl.SphereOCTVolume()
svol.radius=3
svol.center = ocl.Point(5,5,5)

sphvol = camvtk.Sphere(center=(svol.center.x,svol.center.y,svol.center.z), radius=svol.radius)
sphvol.SetColor(camvtk.blue)
sphvol.SetWireframe()
sphvol.SetOpacity(0.3)
myscreen.addActor(sphvol)

cube1 = ocl.CubeOCTVolume()
cube1.side=30
cube1.center = ocl.Point(0,0,0)

cube2 = ocl.CubeOCTVolume()
cube2.center = ocl.Point(0,0,0)
cube2.side = 5

drawTree(myscreen,t,opacity=0.1, color=camvtk.grey)

print " build()"    
t.build(cube1)


print "t.size=", t.size()
#printNodes(t)

#t2 = ocl.LinOCT()
#t2.init(3)
#drawTree(myscreen,t2,opacity=0.1, color=camvtk.grey)
#t2.build(cube2)


drawTree(myscreen,t,opacity=0.3, color=camvtk.red)

#printNodes(t)

t.condense()
print " after condense() ", t.size()
#printNodes(t)
myscreen.render()
time.sleep(1)
drawTree(myscreen,t,opacity=0.3, color=camvtk.green)

t.condense()
print " after SECOND condense() ", t.size()
#printNodes(t)
myscreen.render()
time.sleep(1)
drawTree(myscreen,t,opacity=0.3, color=camvtk.blue)
t.condense()
print " after THIRD condense() ", t.size()
#printNodes(t)
t.condense()
print " after FOURTH condense() ", t.size()
#printNodes(t)

myscreen.render()
myscreen.iren.Start() 
exit()

drawTree(myscreen,t2,opacity=0.3, color=camvtk.green)

print "t2 size=", t2.size()

t2.sum(t)
print " t2.sum(t)=", t2.size()
printNodes(t2)
t2.sort()
print " after sort() ", t2.size()
printNodes(t2)
t2.condense()
print " after condense() ", t2.size()
printNodes(t2)


#drawTree(myscreen,t,opacity=1)
#drawTree(myscreen,t2,opacity=0.3, color=camvtk.green)

#drawTree(myscreen,t2,opacity=0.3, color=camvtk.blue)

#print " nodes:"
#printNodes(t)



myscreen.render()
myscreen.iren.Start() 
    

#t.expand_at(1)
#print "t.size=", t.size()
#printNodes(t)
