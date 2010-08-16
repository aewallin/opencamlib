/*  $Id$
 * 
 *  Copyright 2010 Anders Wallin (anders.e.e.wallin "at" gmail.com)
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


#include <list>
// uncomment to disable assert() calls
// #define NDEBUG
#include <cassert>

#include <boost/python.hpp>
#include <boost/foreach.hpp>

#include "point.h"
#include "triangle.h"
#include "millingcutter.h"
#include "numeric.h"
#include "octree2.h"
#include "volume.h"
#include "mc_tables.h"

namespace ocl
{

//**************** Octree ********************/

Octree::Octree(double scale, unsigned int  depth, Point& centerp) {
    root_scale = scale;
    max_depth = depth;
    //root_center = centerp;
                    // parent, idx, scale, depth
    root = new Octnode( 0, 0, root_scale, 0 );
    root->center = &centerp;
}


/// subdivide the Octree n-times
void Octree::init(unsigned int n) {
    for (unsigned int m=0;m<n;++m) {
        std::vector<Octnode*> nodelist;
        Octree::get_leaf_nodes(root, nodelist);
        //std::cout << " init(m="<<m<<") got "<<nodelist.size()<<" nodes to subdivide\n";
        BOOST_FOREACH( Octnode* n, nodelist) {
            n->subdivide();
        }
    }
}

/// put all leaf nodes into nodelist
void Octree::get_leaf_nodes(Octnode* current, std::vector<Octnode*>& nodelist) const {
    if ( current->leaf == true ) {
        nodelist.push_back( current );
    } else {
        for ( int n=0;n<8;++n) {
            if ( current->child[n] != 0 )
                get_leaf_nodes( current->child[n], nodelist );
        }
    }
}

/// put all nodes into nodelist
void Octree::get_all_nodes(Octnode* current, std::vector<Octnode*>& nodelist) const {
    if ( current ) {
        nodelist.push_back( current );
        for ( int n=0;n<8;++n) {
            if ( current->child[n] != 0 )
                get_all_nodes( current->child[n], nodelist );
        }
    }
}

/// run marching cubes on the whole octree, returning the surface triangles
std::vector<Triangle> Octree::mc() {
    std::vector<Octnode*> leaf_nodes;
    get_leaf_nodes(this->root, leaf_nodes);
    std::cout << " mc() got " << leaf_nodes.size() << " leaf nodes\n";
    std::vector<Triangle> mc_triangles;
    BOOST_FOREACH(Octnode* n, leaf_nodes) {
        std::vector<Triangle> tris = n->mc_triangles();
        BOOST_FOREACH( Triangle t, tris) {
            mc_triangles.push_back(t);
        }
    }
    return mc_triangles;
}

boost::python::list Octree::py_mc_triangles() {
    std::vector<Triangle> mc_triangles = mc();
    boost::python::list tlist;
    BOOST_FOREACH( Triangle t, mc_triangles ) {
        tlist.append( t );
    }
    return tlist;
}

void Octree::diff_negative_root(OCTVolume* vol) {
    diff_negative( this->root, vol);
}

void Octree::diff_negative(Octnode* current, OCTVolume* vol) {

    if ( current->leaf ) {

        current->evaluate( vol ); // this sets the inside/outside flags
        assert( current->idx < 8 );
        if ( current->inside ) { // inside nodes should be deleted
            Octnode* parent = current->parent;
            assert( parent != NULL );
            unsigned int delete_index = current->idx;
            delete parent->child[ delete_index ];
            parent->child[ delete_index ]=0;

            if (parent->leaf)  {// this probably causes segfaulting??
                //assert(0); //FIXME
                Octree::diff_negative( parent, vol ); // then it must be processed
            }
        } else if (current->outside) {// we do nothing to outside nodes.
        } else {// these are intermediate nodes
            if ( current->depth < (this->max_depth-1) ) { // subdivide, if possible
                current->subdivide();
                for(int m=0;m<8;++m) {
                    assert(current->child[m]); // when we subdivide() there must be a child.
                    if ( vol->bb.overlaps( current->child[m]->bb ) )
                        diff_negative( current->child[m], vol); // build child
                }
            } else { 
                // max depth reached, can't subdivide
            }
        }
    } else {

        for(int m=0;m<8;++m) { // not a leaf, so go deeper into tree
            if ( current->child[m] ) {
                if ( vol->bb.overlaps( current->child[m]->bb ) )
                    diff_negative( current->child[m], vol); // build child
            }
        }
    }

}

// search tree and return list of leaf-nodes
boost::python::list Octree::py_get_leaf_nodes() const {
    std::vector<Octnode*> nodelist;
    Octree::get_leaf_nodes(root, nodelist);
    boost::python::list pynodelist;
    BOOST_FOREACH(Octnode* n, nodelist) {
        pynodelist.append( *n );
    }
    return pynodelist;
}


/// string repr
std::string Octree::str() const {
    std::ostringstream o;
    o << " Octree:\n";
    std::vector<Octnode*> nodelist;
    Octree::get_leaf_nodes(root, nodelist);
    std::vector<int> nodelevel(this->max_depth);
    BOOST_FOREACH( Octnode* n, nodelist) {
        ++nodelevel[n->depth];
    }
    o << "  " << nodelist.size() << " leaf-nodes:\n";
    int m=0;
    BOOST_FOREACH( int count, nodelevel) {
        o << "depth="<<m <<" has " << count << " nodes\n";
        ++m;
    }
    return o.str();
}

//**************** Octnode ********************/

// this defines the position of each octree-vertex with relation
// to the center of the node
Point Octnode::direction[8] = {
                     Point( 1, 1,-1),   // 0
                     Point(-1, 1,-1),   // 1
                     Point(-1,-1,-1),   // 2
                     Point( 1,-1,-1),   // 3
                     Point( 1, 1, 1),   // 4
                     Point(-1, 1, 1),   // 5
                     Point(-1,-1, 1),   // 6
                     Point( 1,-1, 1)    // 7
                    };


Octnode::Octnode(Octnode* nodeparent, unsigned int index, double nodescale, unsigned int nodedepth) {
    parent = nodeparent;
    idx = index;
    if (nodeparent)
        center = nodeparent->childcenter(idx);
    else
        center = new Point(0,0,0); // default center for root is (0,0,0)
        
    child.resize(8);
    vertex.resize(8);
    f.resize(8);
    
    scale = nodescale;
    depth = nodedepth;
    leaf = true;
    setvertices();
    mc_tris_valid = false;
    evaluated = false;
}


Octnode::~Octnode() {
    for(int n=0;n<8;++n) {
        if (child[n]) {
            delete child[n];
            child[n] = 0;
        }
        if (vertex[n]) {
            delete vertex[n];
            vertex[n] = 0;
        }
    }
    if (center) {
        delete center;
        center = 0;
    }
}


void Octnode::delete_child(unsigned int index) {
    if ( this->child[index] ) { // if child exists
        //Octnode* c = this->child[index];
        //assert( c );
        //for (int n=0;n<8;++n) {
           // if (c->child[n])
                //c->delete_child(n);
        //}
        delete this->child[index]; // delete
        this->child[index] = 0;
        
        // this might cause all children to have been deleted, thus making this a
        // leaf node again
        bool all_zero = true;
        for( int n=0;n<8;++n ) {
            if ( !(this->child[n] == NULL) )
                all_zero = false;
        }
        if (all_zero) {
            for( int n=0;n<8;++n )
                assert( (this->child[n] == 0) );
            this->leaf = true;
            this->mc_tris_valid = false;
        }
    
    }
    

}

/// create 8 children of this node
void Octnode::subdivide() {
    if (this->leaf) {
        for( int n=0;n<8;++n ) {
                                        // parent,  idx,              scale,   depth
            this->child[n] = new Octnode( this, n , scale/2.0 , depth+1 );
            // inherit the surface property here...
            // optimization: inherit one f[n] from the corner?
        }
        this->leaf = false;
    } else {
        std::cout << " DON'T subdivide a non-leaf node \n";
        assert(0); // DON'T subdivide a non-leaf node
    }
}

void Octnode::evaluate(OCTVolume* vol) {
    assert( leaf );
    outside = true;
    inside = true;
    for ( int n=0;n<8;++n) {
        double newf = vol->dist( *(vertex[n]) );
        if ( !evaluated ) {
            f[n] = newf;
        } else if( (newf < f[n] )   ) {
            f[n] = newf;
            mc_tris_valid = false; 
        } 
        if ( f[n] <= 0.0 ) {// if one vertex is inside
            outside = false; // then it's not an outside-node
        } else { // if one vertex is outside
            assert( f[n] > 0.0 );
            inside = false; // then it's not an inside node anymore
        }
    }
    evaluated = true;
}

/// interpolate f-value between vertices idx1 and idx2
Point Octnode::interpolate(int idx1, int idx2) {
    // p = p1 - v1 (p2-p1)/(v2-v1)
    assert( !isZero_tol( f[idx2]-f[idx1]  ) );
    return *vertex[idx1] - f[idx1]*( *vertex[idx2]-(*vertex[idx1]) ) * 
                                    (1.0/(f[idx2]-f[idx1]));
}

/// use marching-cubes and return a list of triangles for this node
std::vector<Triangle> Octnode::mc_triangles() {
    assert( this->leaf ); // don't call this on non-leafs!
    //assert( !this->inside ); // there should be no inside nodes in the tree!
    std::vector<Triangle> tris;
    if ( this->outside) {
        return tris; // outside nodes do not produce triangles
    } else if (mc_tris_valid) { // if triangles already calculated
        //assert( !mc_tris.empty() ); // FIXME, fails after bb.overlaps() check was added to diff_negative()
        return mc_tris; // return the stored ones
    }
    assert( !this->outside ); // outside nodes will never produce triangles.
    
    unsigned int edgeTableIndex = 0;
    if (f[0] < 0.0 ) edgeTableIndex |= 1;
    if (f[1] < 0.0 ) edgeTableIndex |= 2;
    if (f[2] < 0.0 ) edgeTableIndex |= 4;
    if (f[3] < 0.0 ) edgeTableIndex |= 8;
    if (f[4] < 0.0 ) edgeTableIndex |= 16;
    if (f[5] < 0.0 ) edgeTableIndex |= 32;
    if (f[6] < 0.0 ) edgeTableIndex |= 64;
    if (f[7] < 0.0 ) edgeTableIndex |= 128;
    // the index into this table now tells us which edges have the vertices
    // for the new triangles
    // the lookup returns a 12-bit number, where each bit indicates wether 
    // the edge is cut by the isosurface
    unsigned int edges = edgeTable[edgeTableIndex];
    
    // we should return early (above) from these degenerate cases
    // and not deal with them here
    //assert( edges != 0 );
    //assert( edges != 4096 );

    
    // calculate intersection points by linear interpolation
    // there are now 12 different cases:
    std::vector<Point> vertices(12);
    if ( edges & 1 )
        vertices[0] = interpolate( 0 , 1 );
    if ( edges & 2 )
        vertices[1] = interpolate( 1 , 2 );
    if ( edges & 4 )
        vertices[2] = interpolate( 2 , 3 );
    if ( edges & 8 )
        vertices[3] = interpolate( 3 , 0 );
    if ( edges & 16 )
        vertices[4] = interpolate( 4 , 5 );
    if ( edges & 32 )
        vertices[5] = interpolate( 5 , 6 );
    if ( edges & 64 )
        vertices[6] = interpolate( 6 , 7 );
    if ( edges & 128 )
        vertices[7] = interpolate( 7 , 4 );
    if ( edges & 256 )
        vertices[8] = interpolate( 0 , 4 );
    if ( edges & 512 )
        vertices[9] = interpolate( 1 , 5 );
    if ( edges & 1024 )
        vertices[10] = interpolate( 2 , 6 );
    if ( edges & 2048 )
        vertices[11] = interpolate( 3 , 7 );
    
    // form facets
    for (int i=0; triTable[edgeTableIndex][i] != -1 ; i+=3 ) {
        tris.push_back( Triangle(vertices[ triTable[edgeTableIndex][i  ] ],
                                 vertices[ triTable[edgeTableIndex][i+1] ], 
                                 vertices[ triTable[edgeTableIndex][i+2] ]
                                 ) 
                      );
    }
    this->mc_tris = tris;
    this->mc_tris_valid = true;
    return tris;
}



/// return centerpoint of child with index n
Point* Octnode::childcenter(int n) {
    return  new Point(*center + (0.5*scale * direction[n]));
}

/// set the vertex positions and f[n]=0
void Octnode::setvertices() {
    for ( int n=0;n<8;++n) {
        vertex[n] = new Point(*center + scale*direction[n] ) ;
        f[n] = 1e6;
    }
    bb.clear();
    bb.addPoint( *vertex[2] );
    bb.addPoint( *vertex[4] );
}


Point Octnode::py_get_center() const {
    return *center;
}
 
 
boost::python::list Octnode::py_get_vertices() const {
    boost::python::list vlist;
    for ( int n=0;n<8;++n) 
        vlist.append( *(vertex[n]) );
    return vlist;
}


/// string repr
std::ostream& operator<<(std::ostream &stream, const Octnode &n) {
    stream << " Octnode: N=";     
    return stream;
}


/// string repr
std::string Octnode::str() const {
    std::ostringstream o;
    o << *this;
    return o.str();
}

} // end namespace
// end of file octree2.cpp
