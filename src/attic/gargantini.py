import ocl as cam
import camvtk
import time
import vtk
import math
import datetime

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

def drawNode(myscreen, node):
    if node.type == cam.OCType.BLACK:
        return # don't draw intermediate nodes
    if node.type == cam.OCType.GREY:
        return # don't draw intermediate nodes    
    p = []
    for n in range(1,9):
        p1 = node.nodePoint(n)
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
    
    if node.type == cam.OCType.WHITE:
        color = nodeColor(node)
    if node.type == cam.OCType.GREY:
        color = camvtk.white
    if node.type == cam.OCType.BLACK:
        color = camvtk.grey
        
        
    for li in lines:
        li.SetColor( color )
        if node.type==cam.OCType.BLACK:
            li.SetOpacity(0.1)
        if node.type==cam.OCType.GREY:
            li.SetOpacity(0.2)
        myscreen.addActor(li)
        
def drawNode2(myscreen, node):
    if node.type == cam.OCType.BLACK:
        return # don't draw intermediate nodes
    if node.type == cam.OCType.GREY:
        return # don't draw intermediate nodes    
    p = []
    for n in range(1,9):
        p1 = node.nodePoint(n)
        p.append(p1)
        
    lines = []
    for n in range(0,8):
        lines.append ( camvtk.Point(center=(p[n].x,p[n].y,p[n].z) ) )
        
    if node.type == cam.OCType.WHITE:
        color = nodeColor(node)
    if node.type == cam.OCType.GREY:
        color = camvtk.white
    if node.type == cam.OCType.BLACK:
        color = camvtk.grey
        
        
    for li in lines:
        li.SetColor( color )
        if node.type==cam.OCType.BLACK:
            li.SetOpacity(0.1)
        if node.type==cam.OCType.GREY:
            li.SetOpacity(0.2)
        myscreen.addActor(li)

def drawNode3(myscreen, node):
    if node.type == cam.OCType.BLACK:
        return # don't draw intermediate nodes
    if node.type == cam.OCType.GREY:
        return # don't draw intermediate nodes    
    
    if node.type == cam.OCType.WHITE:
        ccolor = nodeColor(node)
    if node.type == cam.OCType.GREY:
        ccolor = camvtk.white
    if node.type == cam.OCType.BLACK:
        ccolor = camvtk.grey
    cen = node.nodePoint(0)
    cube = camvtk.Cube(center=(cen.x, cen.y, cen.z), length= node.scale, color=camvtk.green)
    #cube.SetWireframe()
    #cube.SetOpacity(0.2)
    myscreen.addActor( cube )
        
def drawOCT(myscreen, oct, color, opacity=1.0):
    nodes = oct.get_white_nodes()
    for node in nodes:
        cen = node.nodePoint(0)
        cube = camvtk.Cube(center=(cen.x, cen.y, cen.z), length= node.scale, color=color)
        cube.SetOpacity(opacity)
        #cube.SetWireframe()
        myscreen.addActor( cube )

class gcode():
    def __init__(self, length=5):
        self.code = []
        self.center=cam.Point(0,0,0)
        
    def nodePoint(self):
        # scan code from left to right
        p = cam.Point(0,0,0)
        for digit in self.code:
            print(digit)

# set operations on linear octrees
# o1 = o2            octant o1 is same as o2
# op1 < op2          octant op1 is smaller than octant op2
# op1 cont op2       octant op1 is contained in op2, implying op1 < op2
# op1 cont/eq op2    octant op1 is contained in or equal to octant op2, implying op1 <= op2
def linoct_setops( o1, o2):
    # initialize results
    union = []
    intersection = []
    diff12 = []
    diff21 = []
    int_ptr=0
    union_ptr=0
    diff12_ptr=0
    diff21_ptr=0
    
    maxLength = max( len(o1), len(o2) )
    
    ptr1 = 0
    ptr2 = 0
    while( (ptr1 <= len(o1)) and (ptr2 <= len(o2)) ):
        if ( o1[ptr1].containedIn( o2[ptr2] ) ): # case1: containment of o1 node in o2 node
            int_ptr += 1
            intersection[int_ptr] = o1[ptr1]
            if ( Hold21 == 0): # difference queue 21 is empty
                Hold21 = o2[ptr2]
            add_to_queue( Q21, o1[ptr1] )
            ptr1 +=1 # move to next in o1
            
        elif ( o2[ptr2].containedIn( o1[ptr1] ) ): # case2: containemnet of o2 node in o1 node
            int_ptr=int_ptr+1
            intersection[int_ptr] = o2[ptr2]
            if ( Hold21 == 0): # difference queue 12 is empty
                Hold12 = o1[ptr1]
            add_to_queue( Q12, o2[ptr2] )
            ptr2 +=1 # move to next in o2
            
        elif ( o1[ptr1].lessThan( o2[ptr2] ) ): # case3: node o1 precedes node of o2
            union_ptr += 1
            union[union_ptr] = o1[ptr1]
            if ( Hold12 == o1[ptr1] ):
                # compute difference o1 - o2
                do_difference( Hold12, Q12, diff12, diff12_ptr )
                Hold12 = 0
            else: # nothing in queue
                diff12_ptr += 1
                diff12[diff12_ptr] = o1[ptr1]
                
            ptr1 +=1    
            
        else:   # case4: node o2 precedes node of o1
            union_ptr +=1
            union[union_ptr] = o2[ptr2]
            if (Hold21 == o2[ptr2]):
                # compute o2 - o1
                do_difference( Hold21, Q21, diff21, diff21_ptr )
                Hold21 = 0
            else: # nothing in queue
                diff21_ptr +=1
                diff21[diff21_ptr] = o2[ptr2]
            ptr2 +=1
        # end while
    # all nodes processed in one list
    # process remaining nodes
    if ( ptr1 < len(o1) ): # process rest of o1
        ptr3 = ptr1
        if (Hold12 == o1[ptr1]):
            do_differences( Hold12, Q12, diff12, diff12_ptr )
            Hold12 = 0
            ptr3 += 1
        for i in range(ptr3, len(o1)):  # difference
            diff12_ptr += 1
            diff12[diff12_ptr] = o1[i]
        for i in range(ptr1, len(o1)):  # union
            union_ptr +=1
            union[union_ptr] = o1[i]
        
    else: # process rest of o2
        ptr3 = ptr2
        if (Hold21 == o2[ptr2]):
            do_differences(Hold21, Q21, diff21, diff21_ptr)
            Hold21=0
            ptr3 +=1
        for i in range(ptr3, len(o2)):
            diff21_ptr +=1
            diff21[diff21_ptr] = o2[i]
        for i in range(ptr2, len(o2)):
            union_ptr += 1
            union[union_ptr] = o2[i]
    
    # union octree is not compressed, so requires compressing here.        
    compress(union, union_ptr)
    # the end
    
def compress( octree, ptr):
    pass
    return

# H - single octant, will be expanded
# Q set of octants contained in octant H stored in dequeue
# D octant, difference-set
# Dptr current index of D
# Hd(Q)  return first node in deque Q (does not remove)
# Tl(Q) return new queue with first node removed
# append(Q1,Q2)   adds Q2 onto end of Q1
# expand(n) takes node and expands into eight suboctants, return deque in ascending order
def do_differences( H, Q, D, Dptr):
    Q2 = expand(H) # create nodes representing suboctants of H
    while Q2:
        if Q:
            n = Hd(Q)
            if ( n==Hd(Q2) ):
                Q2 = Tl(Q2)
                Q = Tl(Q)
            elif ( n.containedIn( Hd(Q2) ) ):
                Q2 = append( expand( Hd(Q2), Tl(Q2) ) )
            else:
                Dptr +=1
                D[Dptr] = Hd(Q2)
                Q2 = Tl(Q2)
        else: # Q is empty
            Dptr +=1
            D[Dptr] = Hd(Q2)
            Q2 = Tl(Q2)
        #end
    #end
    return
    # the end...
    
    
if __name__ == "__main__":  
    #exit()
    #oct = cam.OCTNode()
    myscreen = camvtk.VTKScreen()
    
    myscreen.camera.SetPosition(20, 12, 2)
    myscreen.camera.SetFocalPoint(0,0, 0)
    
    #print oct.str()
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
    
    xar = camvtk.Arrow(color=camvtk.red, rotXYZ=(0,0,0))
    myscreen.addActor(xar)
    yar = camvtk.Arrow(color=camvtk.green, rotXYZ=(0,0,90))
    myscreen.addActor(yar)
    zar = camvtk.Arrow(color=camvtk.blue, rotXYZ=(0,-90,0))
    myscreen.addActor(zar)
    
    oc2 = cam.OCTest()
    oc2.set_max_depth(5)
    
    svol = cam.SphereOCTVolume()
    svol.radius=3.1415
    svol.center = cam.Point(-1,2,-1)
    

    
    oc2.setVol(svol)
    
    oc2.build_octree()
    
    oc3 = cam.OCTest()
    
    
    svol3 = cam.SphereOCTVolume()
    svol3.radius=2
    svol3.center = cam.Point(-1,2,1)
    
    cvol = cam.CubeOCTVolume()
    cvol.side = 3
    cvol.center = cam.Point(2.0,2,-1)
    
    
    oc3.setVol(cvol)
    oc3.set_max_depth(5)
    oc3.build_octree()
    iters = oc3.prune_all()
    
    iters = oc2.prune_all()
    nlist = oc2.get_all_nodes()
    print(" oc2 got ", len(nlist), " nodes")
    nlist = oc2.get_white_nodes()
    print(" oc2 got ", len(nlist), " white nodes")
    nlist = oc3.get_all_nodes()
    print(" oc3 got ", len(nlist), " nodes")
    
    

    
    print("calling balance")
    oc2.balance(oc3)
    print("after balance:")
    nlist = oc2.get_all_nodes()
    print(" oc2 got ", len(nlist), " nodes")
    nlist = oc2.get_white_nodes()
    print(" oc2 got ", len(nlist), " white nodes")
    print("calling diff")
    oc2.diff(oc3)
    print("after diff:")
    nlist = oc2.get_all_nodes()
    print(" oc2 got ", len(nlist), " nodes")
    nlist = oc2.get_white_nodes()
    print(" oc2 got ", len(nlist), " white nodes")
    
    drawOCT(myscreen, oc2, camvtk.green)
    #drawOCT(myscreen, oc3, camvtk.red, opacity=0.1)
    #exit()
    #for node in nlist2:
    #    pass
        #print node.str()
        #p1 = node.nodePoint(0)    
     #   drawNode3( myscreen, node )
        #myscreen.addActor( camvtk.Sphere(center=(p1.x, p1.y, p1.z), radius=0.1, color=sph_color))
    
    
    
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
    
