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

#ifndef KDTREE_H
#define KDTREE_H

#include <iostream>
#include <list>
#include "point.h"
#include "triangle.h"

///
/// \brief K-D tree
/// http://en.wikipedia.org/wiki/Kd-tree
///
/// this is also briefly explained in a paper by Yau et al. 
/// http://dx.doi.org/10.1080/00207540410001671651

class Spread {
    public:
        Spread(int dim, double v, double s);
        int d;
        double val;
        double start;
        int sp_comp(Spread x, Spread y);
};


class KDNode {
    public:
        KDNode(int d, double cv, KDNode *hi_c, 
               KDNode *lo_c, std::list<Triangle> *tlist);
        std::string str();
        
        
        /// dimension of cut
        int dim;
        /// cut value
        double cutval;
        /// hi child
        KDNode *hi;
        /// lo child
        KDNode *lo;
        /// list of triangles, if this is a bucket-node
        std::list<Triangle> *tris;
};


class KDTree {
    public:
        static KDNode* build_kdtree(std::list<Triangle> *tris);
        static Spread* spread(std::list<Triangle> *tris);
        static void search_kdtree(std::list<Triangle> *tris, Point &p, 
                    MillingCutter &c, KDNode *node);
        static void str();
};

#endif
