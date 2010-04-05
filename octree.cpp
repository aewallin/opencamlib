/*  Copyright 2010 Anders Wallin (anders.e.e.wallin "at" gmail.com)
 *  
 *  This file is part of OpenCAMlib.
 *
 *  OpenCAMlib is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  OpenCAMlib is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with OpenCAMlib.  If not, see <http://www.gnu.org/licenses/>.
*/

// this is mostly a translation to c++ of the earlier c# code
// http://code.google.com/p/monocam/source/browse/trunk/Project2/monocam_console/monocam_console/kdtree.cs

#include <iostream>
#include <stdio.h>
#include <sstream>
#include <math.h>
#include <vector>
#include <algorithm>

#include "cutter.h"
#include "point.h"
#include "triangle.h"
#include "cutter.h"
#include "numeric.h"

#include "octree.h"



//********   OCTNode ********************** */

Point maxextent = Point(10,10,10);
Point minextent = Point(-10,-10,-10);

OCTNode::OCTNode()
{
    level = 0;
    center = Point(0,0,0);
    type = WHITE;
    parent = NULL;
}

OCTNode::OCTNode(int level, Point &center, OCType type, KDNode *parent,
                                std::vector<OCTNode*> childvec) 
{
    return;
}

OCTNode* OCTNode::build_octree()
{
    return NULL;
}



//********  string output ********************** */
std::string OCTNode::str()
{
    std::ostringstream o;
    o << "OCTNode l="<< level << " center=" << center << " type="<< type <<"\n";
    return o.str();
}

std::ostream& operator<<(std::ostream &stream, const OCTNode root)
{
    stream << "OCTNode";    
    return stream;
}

// end of file octree.cpp
