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
#include <iostream>
#include <stdio.h>
#include <sstream>
#include <math.h>
#include <boost/progress.hpp>
#include <vector>
#include <algorithm>

#include "cutter.h"
#include "point.h"
#include "triangle.h"
#include "cutter.h"
#include "numeric.h"
#include "kdtree.h"



//********   KDNode ********************** */


KDNode::KDNode(int d, double cv, KDNode *hi_c, KDNode *lo_c,
            std::list<Triangle> *tlist) {
    dim = d;
    cutval = cv;
    hi = hi_c;
    lo = lo_c;
    tris = tlist;
}

//** KDTREE

/// given a list of triangles, build and return a kd-tree
/// returns the root of the kd-tree
KDNode* KDTree::build_kdtree(std::list<Triangle> *tris) {
    if (tris->size() == 0) {
        std::cout << "kdtree.cpp ERROR: build_kdtree called with tris->size()==0 ! \n";
        return 0;
    }
    
    // if triangles contained within sufficiently small rectangle (FIXME)
    // OR number of triangles is smaller than some MAX_BUCKET_SIZE (FIXME)
    // OR only one triangle remains 
    // then return a bucket node
    if (tris->size() == 1) {
        KDNode *bucket_node = new KDNode(0, 0, NULL, NULL, tris);
        return bucket_node;
    }
    
    // calculate spread in order to know how to cut
    Spread* spr = KDTree::spread(tris);
    
    // if max spread is 0, return bucket node (?when does this happen?)
    
    // select along which dimension to cut
    
    // build lists of triangles for hi and lo child nodes
    
    // return a new node
    return 0;
}


int spread_compare(Spread x, Spread y) {
    if (x.val > y.val)
        return 1;
    if (y.val > x.val)
        return -1;
    else
        return 0;
}

/// find the maximum 'extent' of triangles in list tris along dimension d
Spread* KDTree::spread(std::list<Triangle> *tris) {
    double max_xplus=0, min_xplus=0, max_xminus=0, min_xminus=0;
    double max_yplus = 0, min_yplus = 0, max_yminus = 0, min_yminus = 0;
    double spr_xplus = 0, spr_xminus = 0, spr_yplus = 0, spr_yminus = 0;
    
    if (tris->size() == 0)
        return new Spread(0,0,0);
    else {
        int n=1;
        BOOST_FOREACH(Triangle t, *tris) {
            if (n==1) {
                // initialize things on the first run
                max_xplus  = t.maxx;
                min_xplus  = t.maxx;
                max_xminus = t.minx;
                min_xminus = t.minx;

                max_yplus  = t.maxy;
                min_yplus  = t.maxy;
                max_yminus = t.miny;
                min_yminus = t.miny;
            } // end initialize
            else { // FIXME madness really, but for only 4 dimensions this works...
            
                // compute spread in xplus
                if (t.maxx > max_xplus)
                    max_xplus = t.maxx;
                if (t.maxx < min_xplus)
                    min_xplus = t.maxx;
                    
                // compute spread in xminus
                if (t.minx > max_xminus)
                    max_xminus = t.minx;
                if (t.minx < min_xminus)
                    min_xminus = t.minx;
                
                // compute spread in yplus
                if (t.maxy > max_yplus)
                    max_yplus = t.maxy;
                if (t.maxy < min_yplus)
                    min_yplus = t.maxy;
                    
                // compute spread in yminus
                if (t.miny > max_yminus)
                    max_yminus = t.miny;
                if (t.miny < min_yminus)
                    min_yminus = t.miny;



            }
            ++n;
        } // end FOREACH triangle in tris
        
        // calculate the spread along each dimension
        spr_xplus = max_xplus - min_xplus;
        spr_xminus = max_xminus - min_xminus;
        spr_yplus = max_yplus - min_yplus;
        spr_yminus = max_yminus - min_yminus;
        
        // put the spreads in a list
        std::vector<Spread*> spreads;
        // = new std::vector<Spread*>();
        spreads.push_back(new Spread(0, spr_xplus, min_xplus));
        spreads.push_back(new Spread(1, spr_xminus, min_xminus));
        spreads.push_back(new Spread(2, spr_yplus, min_yplus));
        spreads.push_back(new Spread(3, spr_yminus, min_yminus));
        // sort the list
        std::sort(spreads.begin(), spreads.end(), spread_compare);
        // spreads->sort(Spread::sp_comp);
        // select the biggest spread and return
        return spreads[ spreads.size()-1 ];
        
    } // end tris->size != 0

} // end spread()

//*********** Spread ****************
int Spread::sp_comp(Spread x, Spread y) {
    if (x.val > y.val)
        return 1;
    if (y.val > x.val)
        return -1;
    else
        return 0;
}



//********  string output ********************** */
std::string KDNode::str()
{
    std::ostringstream o;
    o << "KDNode\n";
    return o.str();
}

std::ostream& operator<<(std::ostream &stream, KDNode root)
{
    if (root.tris) {
        if (root.tris->size() > 0)
            stream << "Bucket node with"<< root.tris->size() <<" triangles\n";
    }
    if (root.hi)
        stream << *(root.hi);
    if (root.lo)
        stream << *(root.lo);
    return stream;
}

