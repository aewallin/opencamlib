import ocl as cam
import camvtk
import time
import vtk
import math
import datetime

red= (1,0,0)
green= (0,1,0)
blue= (0,0,1)
cyan=  (0,1,1)
yellow= (1,1,0)
pink = ( float(255)/255,float(192)/255,float(203)/255)
grey = ( float(127)/255,float(127)/255,float(127)/255)
orange = ( float(255)/255,float(165)/255,float(0)/255)


#OCType = Enum('black', 'grey', 'white')
OCTMax = 8

def buildOCTree(volume, nodecenter=cam.Point(0,0,0), level=0):
    # build octree of volume, return root node
    
    node = OCTNode( level, center = nodecenter , type = 1, childlist=None)
    
    flags = []
    for n in range(0,9): # test all points
        flags.append( volume.isInside( node.nodePoint(n) ) )
    
    if (sum(flags) == 0): # nothing is inside
        node.type = 0
        #print "nothing inside!"
        return node
    
    if (sum(flags) == 9): # everything is inside
        node.type = 2
        #print "all inside!"
        return node
        
    if level== OCTMax: # reached max levels
        return node #OCTNode(level, center= nodecenter, type = 2, childlist = None)
    
    
    # have to subdivide:
    childs = []
    child_centers = []
    for n in range(1,9):
        child_center = node.childCenter(n) 
        childs.append( buildOCTree( volume , nodecenter = child_center, level= level+1) )
    node.setChildren(childs)
    
    return node

def searchOCTree(node, list):
    # return list of nodes in the whole tree starting at node
    
    if node.children is not None:
        for chi in node.children:
            searchOCTree(chi, list)
    else:
        list.append(node)

class Volume():
    def __init__(self):
        self.center = cam.Point(0,0,0)
        self.radius = 0.45
        
    def isInside(self, point):
        p = point - self.center
        if p.norm() < self.radius:
            return 1
        else:
            return 0

class OCTNode():
    def __init__(self, level=0, center=cam.Point(0,0,0), type = None, childlist=[]):
        self.level = level
        self.center = cam.Point(center)
        self.scale = float(1) / (2**level)
        self.children = childlist
        self.type = type
    
    def setChildren(self, list):
        self.children = list
    

    
    def posDir(self, index):
        if index==0:
            return cam.Point(0,0,0)
        if index==1:
            return cam.Point(1,1,1)
        if index==2:
            return cam.Point(-1,1,1)
        if index==3:
            return cam.Point(1,-1,1)
        if index==4:
            return cam.Point(1,1,-1)
        if index==5:
            return cam.Point(1,-1,-1)
        if index==6:
            return cam.Point(-1,1,-1)
        if index==7:
            return cam.Point(-1,-1,1)
        if index==8:
            return cam.Point(-1,-1,-1)
    
    def nodePoint(self, index):
        return self.center + 1.0 * self.scale * self.posDir(index)
    
    def childCenter(self, index):
        return self.center + 0.5 * self.scale * self.posDir(index)

def nodeColor(oct):
    offset = 2
    n = oct.level-offset
    return (float(n)/(OCTMax-offset), float(OCTMax-offset - n)/(OCTMax-offset), 0)

def addNodes(myscreen, oct):
    if oct.type == 1:
        return # don't draw intermediate nodes
        
    p = []
    for n in range(1,9):
        p1 = oct.nodePoint(n)
        p.append(p1)
        
    lines = []
    lines.append ( camvtk.Line(p1=(p[0].x,p[0].y,p[0].z),p2=(p[1].x,p[1].y,p[1].z)) )
    lines.append ( camvtk.Line(p1=(p[0].x,p[0].y,p[0].z),p2=(p[2].x,p[2].y,p[2].z)) )
    lines.append ( camvtk.Line(p1=(p[0].x,p[0].y,p[0].z),p2=(p[3].x,p[3].y,p[3].z)) )
    lines.append ( camvtk.Line(p1=(p[2].x,p[2].y,p[2].z),p2=(p[4].x,p[4].y,p[4].z)) )
    lines.append ( camvtk.Line(p1=(p[1].x,p[1].y,p[1].z),p2=(p[5].x,p[5].y,p[5].z)) )
    lines.append ( camvtk.Line(p1=(p[1].x,p[1].y,p[1].z),p2=(p[6].x,p[6].y,p[6].z)) )
    lines.append ( camvtk.Line(p1=(p[2].x,p[2].y,p[2].z),p2=(p[6].x,p[6].y,p[6].z)) )
    lines.append ( camvtk.Line(p1=(p[6].x,p[6].y,p[6].z),p2=(p[7].x,p[7].y,p[7].z)) )
    lines.append ( camvtk.Line(p1=(p[4].x,p[4].y,p[4].z),p2=(p[7].x,p[7].y,p[7].z)) )
    lines.append ( camvtk.Line(p1=(p[4].x,p[4].y,p[4].z),p2=(p[3].x,p[3].y,p[3].z)) )
    lines.append ( camvtk.Line(p1=(p[5].x,p[5].y,p[5].z),p2=(p[3].x,p[3].y,p[3].z)) )
    lines.append ( camvtk.Line(p1=(p[5].x,p[5].y,p[5].z),p2=(p[7].x,p[7].y,p[7].z)) )
    
    if oct.type == 0:
        color = camvtk.grey
    if oct.type == 1:
        color = camvtk.green
    if oct.type == 2:
        color = nodeColor(oct)
        
        
    for li in lines:
        li.SetColor( color )
        if oct.type==0:
            li.SetOpacity(0.2)
        myscreen.addActor(li)
        

        
if __name__ == "__main__":  
    myscreen = camvtk.VTKScreen()
    
    myscreen.camera.SetPosition(5, 3, 2)
    myscreen.camera.SetFocalPoint(0,0, 0)
    
    xar = camvtk.Arrow(color=red, rotXYZ=(0,0,0))
    #myscreen.addActor(xar)
    yar = camvtk.Arrow(color=green, rotXYZ=(0,0,90))
    #myscreen.addActor(yar)
    zar = camvtk.Arrow(color=blue, rotXYZ=(0,-90,0))
    #myscreen.addActor(zar)
    
    oct = OCTNode(level=0)
    
    testvol = Volume()
    
    print("building tree...",)
    tree = buildOCTree(testvol)
    print("done.")
    print(tree)
    
    list =[]
    searchOCTree(tree, list)
    print(len(list), " nodes in tree")
    
    w2if = vtk.vtkWindowToImageFilter()
    w2if.SetInput(myscreen.renWin)
    lwr = vtk.vtkPNGWriter()
    lwr.SetInput( w2if.GetOutput() )
    w2if.Modified()

    
    t = camvtk.Text()
    t.SetPos( (myscreen.width-200, myscreen.height-30) )
    myscreen.addActor( t)
    
    t2 = camvtk.Text()
    t2.SetPos( (myscreen.width-200, 30) )
    myscreen.addActor( t2)
    
    n = 0
    for node in list:
        addNodes(myscreen, node)
        if (n%50) == 0:
            nodetext = "Nodes: %5i" % (n)
            t2.SetText(nodetext)
            t.SetText("OpenCAMLib 10.03-beta " + datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S"))
            myscreen.render()
            myscreen.camera.Azimuth( 3 )
            print("frame %i of %i" % (n, len(list)))
            w2if.Modified() 
            lwr.SetFileName("frames/oct"+ ('%05d' % n)+".png")
            #lwr.Write()

        n = n +1
        #time.sleep(0.1)
        

    
    print("done!")
    
    myscreen.render()
    
    myscreen.iren.Start()
    #raw_input("Press Enter to terminate") 
    
