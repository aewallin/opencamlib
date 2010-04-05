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


double  OCTNode::max_scale = 10.0;

double OCTNode::get_max_scale()
{
    return max_scale;
}

OCTNode::OCTNode()
{
    level = 0;
    scale = max_scale/pow(2.0, level);
    
    center = Point(0,0,0);
    type = WHITE;
    parent = NULL;
}

OCTNode::OCTNode(int level, Point &center, OCType type, KDNode *parent,
                                std::vector<OCTNode*> childvec) 
{
    return;
}

/// return a node-point    
Point OCTNode::nodePoint(int id)
{
    return center + scale*nodeDir(id);
}

/// return center-point of child
Point OCTNode::childCenter(int id)
{
    return center + (scale/2)*nodeDir(id);
}
        
/// return direction to node-point
Point OCTNode::nodeDir(int id)
{
    switch(id)
    {
        case 0:
            return Point(0,0,0);
            break;
        case 1:
            return Point(1,1,1);
            break;
        case 2:
            return Point(-1,1,1);
            break;
        case 3:
            return Point(1,-1,1);
            break;
        case 4:
            return Point(1,1,-1);
            break;
        case 5:
            return Point(1,-1,-1);
            break;
        case 6:
            return Point(-1,1,-1);
            break;
        case 7:
            return Point(-1,-1,1);
            break;
        case 8:
            return Point(-1,-1,-1);
            break;
        default:
            std::cout << "octree.cpp nodeDir() called with invalid id!!\n";
            assert(0);
            break;
    }
    assert(0);
    return Point(0,0,0);
}
        

        
        

OCTNode* OCTNode::build_octree()
{
    return NULL;
}



//********  string output ********************** */
std::string OCTNode::str()
{
    std::ostringstream o;
    o << "OCTNode l="<< level << " center=" << center << " scale=" << scale << " type="<< type;
    return o.str();
}

std::ostream& operator<<(std::ostream &stream, const OCTNode root)
{
    stream << "OCTNode";    
    return stream;
}

// end of file octree.cpp
