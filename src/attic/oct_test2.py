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
    
    oct = cam.OCTNode()
    myscreen = camvtk.VTKScreen()
    
    myscreen.camera.SetPosition(5, 3, 2)
    myscreen.camera.SetFocalPoint(0,0, 0)
    
    print(oct.str())
    """
    print("max scale=", oct.get_max_scale())
    for n in range(0,9):
        p1 = oct.nodePoint(n)
        myscreen.addActor( camvtk.Sphere(center=(p1.x, p1.y, p1.z), radius=0.1, color=camvtk.red))
        print("id=%i" % (n),)
        print(p1.str())
    
    print("child centers:")
    for n in range(1,9):
        p1 = oct.childCenter(n)
        myscreen.addActor( camvtk.Sphere(center=(p1.x, p1.y, p1.z), radius=0.1, color=camvtk.yellow))
        print("id=%i" % (n),)
        print(p1.str())
        
    """
    
    xar = camvtk.Arrow(color=red, rotXYZ=(0,0,0))
    myscreen.addActor(xar)
    yar = camvtk.Arrow(color=green, rotXYZ=(0,0,90))
    myscreen.addActor(yar)
    zar = camvtk.Arrow(color=blue, rotXYZ=(0,-90,0))
    myscreen.addActor(zar)
    
    oc2 = cam.OCTest()
    oc2.build_octree()
    
    nlist = oc2.get_all_nodes()
    
    print("got ", len(nlist), " nodes")
    
    for node in nlist:
        print(node.str())
        if node.type == cam.OCType.WHITE:
            sph_color = camvtk.white
        if node.type == cam.OCType.BLACK:
            sph_color = camvtk.red
        p1 = node.nodePoint(0)    
        myscreen.addActor( camvtk.Sphere(center=(p1.x, p1.y, p1.z), radius=0.1, color=sph_color))
    
    myscreen.render()
    myscreen.iren.Start() 
    exit()
    #oct = OCTNode(level=0)
    
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
    

    #raw_input("Press Enter to terminate") 
    
