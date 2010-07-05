"""@camvtk docstring
This module provides helper classes for testing and debugging OCL
This module is part of OpenCAMLib (ocl), a toolpath-generation library.
"""

#import vtk
#import time
#import datetime
import ocl
import math

def CLPointGrid(minx,dx,maxx,miny,dy,maxy,z):
    """ generate and return a rectangular grid of points """
    plist = []
    xvalues = [round(minx+n*dx,2) for n in xrange(int(round((maxx-minx)/dx))+1) ]
    yvalues = [round(miny+n*dy,2) for n in xrange(int(round((maxy-miny)/dy))+1) ]
    for y in yvalues:
        for x in xvalues:
            plist.append( ocl.CLPoint(x,y,z) )
    return plist
    

