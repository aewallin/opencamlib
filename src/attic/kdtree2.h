/*  $Id$
 * 
 *  Copyright (c) 2010 Anders Wallin (anders.e.e.wallin "at" gmail.com).
 *  
 *  This file is part of OpenCAMlib 
 *  (see https://github.com/aewallin/opencamlib).
 *  
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 2.1 of the License, or
 *  (at your option) any later version.
 *  
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *  
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef KDTREE2_H
#define KDTREE2_H

#include <iostream>
#include <list>
#include "clpoint.h"

namespace ocl
{
    
class Point;
class Triangle;
class MillingCutter;

/// \brief KDTree spread, a measure of how spread-out a list of triangles are.
///
/// simple struct-like class for storing the "spread" or maximum 
/// extent of a list of triangles. Used by the kd-tree algorithm.
class Spread2 {
    public:
        /// constructor
        Spread2(int dim, double v, double s);
        /// dimension
        int d;
        /// spread-value
        double val;
        /// minimum or start value
        double start;
        /// comparison of Spread objects. Used for finding the largest spread
        /// along which the next partition/cut is made.
        static bool spread_compare(Spread2 *x, Spread2 *y);
};

/// \brief K-D tree node. http://en.wikipedia.org/wiki/Kd-tree
///
/// A k-d tree is used for searching for triangles overlapping with the cutter.
///
/// Using a kd-tree with drop-cutter this is mentioned in a paper by Yau et al. 
/// http://dx.doi.org/10.1080/00207540410001671651
class KDNode2 {
    public:
        /// Create a node which partitions(cuts) along dimension d, at 
        /// cut value cv, with child-nodes hi_c and lo_c.
        /// If this is a bucket-node containing triangles, 
        /// they are in the list tris
        /// lev indicates the level of the node in the tree
        KDNode2(int d, double cv, KDNode2 *up_c,
                                  KDNode2 *hi_c, 
                                  KDNode2 *lo_c, 
                                  const std::list<Triangle> *tlist, 
                                  int level);
        /// string repr
        std::string str() const;
        /// string repr
        friend std::ostream &operator<<(std::ostream &stream, const KDNode2 node);

        /// level of node in tree 
        int level;
        /// dimension of cut or partition.
        int dim;
        /// Cut or partition value.
        /// Child node hi should contain only triangles with a higher value than this.
        /// Child node lo contains triangles with lower values.
        double cutval;
        
        /// parent-node
        KDNode2 *up; 
        /// Child-node hi.
        KDNode2 *hi; 
        /// Child-node lo.
        KDNode2 *lo; 

        /// A list of triangles, if this is a bucket-node
        const std::list<Triangle> *tris;

        /* static functions to build and search KD-trees) */
        /// build a kd-tree from a list of triangles. return root of tree.
        static KDNode2* build_kdtree(const std::list<Triangle> *tris, 
                                     unsigned int bucketSize = 1,
                                     int level = 0,
                                     KDNode2 *parent=NULL);

        /// calculate along which dimension kd-tree should cut
        static Spread2* spread(const std::list<Triangle> *tris);

        /// search KDTree, starting at KDNode root for triangles under the 
        /// MillingCutter c positioned at Point cl.
        /// The triangles found are pushed into the Triangle list tris.
        static void search_kdtree( std::list<Triangle> *tris, 
                                   const Bbox& bb, 
                                   KDNode2 *root,
                                   const unsigned int plane = 0);
        
        /// undocumented, related to selecting the search plane...
        static bool plane_valid(const unsigned int dim,const unsigned int plane);
        
        /// find all triangles under node
        static void getTriangles( std::list<Triangle> *tris, 
                                              KDNode2 *root);

        /// do the triangles at KDNode root overlap (in the XY-plane) with the 
        /// MillingCutter c positioned at Point cl?
        static bool overlap(const KDNode2 *root,      // root of tree
                            const CLPoint& cl,       // cutter positioned here
                            const MillingCutter& c); // cutter

        /// counter used for cutting along the dims sequentially.
        /// not used if cutting along max spread.
        static int cutcount;        
};

} // end namespace
#endif
// end file kdtree2.h
