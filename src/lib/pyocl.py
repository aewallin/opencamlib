"""@camvtk docstring
This module provides helper classes for testing and debugging OCL
This module is part of OpenCAMLib (ocl), a toolpath-generation library.

Copyright 2010-2011 Anders Wallin (anders.e.e.wallin "at" gmail.com)
Published under the GNU General Public License, see http://www.gnu.org/licenses/
"""

#import vtk
#import time
#import datetime
import ocl
import math

def CLPointGridZigZag(minx,dx,maxx,miny,dy,maxy,z):
    """ generate and return a zigzag grid of points """
    plist = []
    xvalues = [round(minx+n*dx,2) for n in xrange(int(round((maxx-minx)/dx))+1) ]
    yvalues = [round(miny+n*dy,2) for n in xrange(int(round((maxy-miny)/dy))+1) ]
    
    #yrow = 0
    #x=minx
    #dir = 0
    xlist = xvalues
    for y in yvalues:
        #xlist = xvalues
        #if dir == 1:
        #    xlist.reverse()
        #    dir = 0
        #else:
        #    dir = 1
            
        for x in xlist:
            plist.append( ocl.CLPoint(x,y,z) )
        xlist.reverse()
        #yrow=yrow+1
    return plist
    

def CLPointGrid(minx,dx,maxx,miny,dy,maxy,z):
    """ generate and return a rectangular grid of points """
    plist = []
    xvalues = [round(minx+n*dx,2) for n in xrange(int(round((maxx-minx)/dx))+1) ]
    yvalues = [round(miny+n*dy,2) for n in xrange(int(round((maxy-miny)/dy))+1) ]
    for y in yvalues:
        for x in xvalues:
            plist.append( ocl.CLPoint(x,y,z) )
    return plist
    
def octree2trilist(t):
    """ return a list of triangles corresponding to the input octree """
    nodes = t.get_nodes()
    tlist = []
    for n in nodes:
        p1 = n.corner(0) # + + +
        p2 = n.corner(1) # - + +
        p3 = n.corner(2) # + - +
        p4 = n.corner(3) # + + -
        p5 = n.corner(4) # + - -
        p6 = n.corner(5) # - + -
        p7 = n.corner(6) # - - +
        p8 = n.corner(7) # - - -
        tlist.append(ocl.Triangle(p1,p2,p3)) #top
        tlist.append(ocl.Triangle(p2,p3,p7)) #top
        tlist.append(ocl.Triangle(p4,p5,p6)) # bot
        tlist.append(ocl.Triangle(p5,p6,p8)) # bot
        tlist.append(ocl.Triangle(p1,p3,p4)) # 1,3,4,5
        tlist.append(ocl.Triangle(p4,p5,p3))
        tlist.append(ocl.Triangle(p2,p6,p7)) # 2,6,7,8
        tlist.append(ocl.Triangle(p7,p8,p6))
        tlist.append(ocl.Triangle(p3,p5,p7)) # 3,5,7,8
        tlist.append(ocl.Triangle(p7,p8,p5))
        tlist.append(ocl.Triangle(p1,p2,p4)) # 1,2,4,6
        tlist.append(ocl.Triangle(p4,p6,p2))
    return tlist
