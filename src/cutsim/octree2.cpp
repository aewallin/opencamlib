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
                    // parent, idx, scale, depth
    root = new Octnode( 0, 0, root_scale, 0 );
    root->center = &centerp;
    
    for (int m=0;m<6;++m) // the root node has all cube-surfaces
        root->surface[m]=true;
}


/// subdivide the Octree n-times
void Octree::init(const unsigned int n) {
    for (unsigned int m=0;m<n;++m) {
        std::vector<Octnode*> nodelist;
        Octree::get_leaf_nodes(root, nodelist);
        BOOST_FOREACH( Octnode* node, nodelist) {
            node->subdivide();
        }
    }
}

/// put leaf nodes into nodelist
void Octree::get_leaf_nodes(Octnode* current, std::vector<Octnode*>& nodelist) const {
    if ( current->childcount == 0 ) {
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

/// generate side_triangles
std::vector<Triangle> Octree::side_triangles() {
    std::vector<Octnode*> leaf_nodes;
    get_leaf_nodes(this->root, leaf_nodes);
    std::cout << " Octree::side_triangles() got " << leaf_nodes.size() << " leaf nodes\n";
    std::vector<Triangle> s_triangles;
    BOOST_FOREACH(Octnode* n, leaf_nodes) {
        std::vector<Triangle> tris = n->side_triangles();
        BOOST_FOREACH( Triangle t, tris) {
            s_triangles.push_back(t);
        }
    }
    return s_triangles;
}

boost::python::list Octree::py_mc_triangles() {
    std::vector<Triangle> mc_triangles = mc();
    boost::python::list tlist;
    BOOST_FOREACH( Triangle t, mc_triangles ) {
        tlist.append( t );
    }
    return tlist;
}

boost::python::list Octree::py_s_triangles() {
    std::vector<Triangle> s_triangles = side_triangles();
    boost::python::list tlist;
    BOOST_FOREACH( Triangle t, s_triangles ) {
        tlist.append( t );
    }
    return tlist;
}

void Octree::diff_negative_root(const OCTVolume* vol) {
    diff_negative( this->root, vol);
}

void Octree::diff_negative(Octnode* current, const OCTVolume* vol) {
    current->evaluate( vol ); // this sets the inside/outside flags
    if ( current->childcount == 0) { // process only leaf-nodes
        if ( current->inside ) { // inside nodes should be deleted
            Octnode* parent = current->parent;
            assert( parent );
            if (parent) {
                unsigned int delete_index = current->idx;
                assert( delete_index >=0 && delete_index <=7 ); 
                delete parent->child[ delete_index ];
                parent->child[ delete_index ]=0;
                --parent->childcount;
                assert( parent->childcount >=0 && parent->childcount <=8);
                
                if (parent->childcount == 0)  { // if the parent has become a leaf node
                    parent->evaluate( vol );
                    assert( parent->inside  ); // then it is itself inside
                }
            }
        } else if (current->outside) {// we do nothing to outside nodes.
        } else {// these are intermediate nodes
            if ( current->depth < (this->max_depth-1) ) { // subdivide, if possible
                current->subdivide();
                assert( current->childcount == 8 );
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

// this defines the position of each octree-vertex with relation to the center of the node
// this also determines in which direction the center of a child node is
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
// surface enumeration
// surf     vertices  vertices
// 0:       2,3,7     2,6,7
// 1:       0,4,7     0,3,7
// 2:       0,1,4     1,4,5
// 3:       1,5,6     1,2,6
// 4:       0,2,3     0,1,2
// 5:       4,6,7     4,5,6


Octnode::Octnode(Octnode* nodeparent, unsigned int index, double nodescale, unsigned int nodedepth) {
    parent = nodeparent;
    idx = index;
    child.resize(8);
    vertex.resize(8);
    f.resize(8);
    surface.resize(6);
    
    if (parent) {
        center = parent->childcenter(idx);
        set_surfaces();
        inside = parent->inside;
        outside = parent->outside;
    } else {
        outside = true;
        inside = false;
        center = new Point(0,0,0); // default center for root is (0,0,0)
    }
    scale = nodescale;
    depth = nodedepth;
    setvertices();
    mc_tris_valid = false;
    evaluated = false;
    childcount = 0;

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

void Octnode::print_surfaces() {
    for (int n=0;n<6;++n)
        std::cout << surface[n];
    std::cout << "\n";
}

void Octnode::set_surfaces() {
    assert( parent );
    surface = parent->surface; // copy surface flags from parent
    // switch(parent index)
    //std::cout << "set_surfaces() on idx="<<idx<< " before= ";
    
    //print_surfaces();
    
    switch ( this->idx ) {
        case 0:
            surface[0]=false;
            surface[3]=false;
            surface[5]=false;
            break;
        case 1:
            surface[0]=false;
            surface[1]=false;
            surface[5]=false;
            break;
        case 2:
            surface[1]=false;
            surface[2]=false;
            surface[5]=false;
            break;
        case 3:
            surface[2]=false;
            surface[3]=false;
            surface[5]=false;
            break;
        case 4:
            surface[0]=false;
            surface[3]=false;
            surface[4]=false;
            break;
        case 5:
            surface[0]=false;
            surface[1]=false;
            surface[4]=false;
            break;
        case 6:
            surface[1]=false;
            surface[2]=false;
            surface[4]=false;
            break;
        case 7:
            surface[2]=false;
            surface[3]=false;
            surface[4]=false;
            break;
        default:
            assert(0);
    }
    //std::cout << " after=";
    //print_surfaces();
}

/// create 8 children of this node
void Octnode::subdivide() {
    if (this->childcount==0) {
        for( int n=0;n<8;++n ) {
                                        // parent,  idx,              scale,   depth
            this->child[n] = new Octnode( this, n , scale/2.0 , depth+1 );
            ++childcount;
            // inherit the surface property here
            
            // optimization: inherit one f[n] from the corner?
        }
    } else {
        std::cout << " DON'T subdivide a non-leaf node \n";
        assert(0); 
    }
}

void Octnode::evaluate(const OCTVolume* vol) {
    //assert( childcount==0 );
    outside = true;
    inside = true;
    for ( int n=0;n<8;++n) {
        double newf = vol->dist( *(vertex[n]) );
        if ( !evaluated ) {
            f[n] = newf;
            mc_tris_valid = false; 
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
    assert( this->childcount == 0 ); // don't call this on non-leafs!
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

std::vector<Triangle> Octnode::side_triangles() {
    assert( this->childcount == 0 ); // this is a leaf-node
    std::vector<Triangle> tris;
    //std::cout << " side_tris() : ";
    //print_surfaces();
    //if ( this->outside) {
        Point vertices[12];
        vertices[0] = *vertex[0];
        vertices[1] = *vertex[1];
        vertices[2] = *vertex[2];
        vertices[3] = *vertex[3];
        vertices[4] = *vertex[4];
        vertices[5] = *vertex[5];
        vertices[6] = *vertex[6];
        vertices[7] = *vertex[7];
        if(surface[0] == true) {
            tris.push_back( Triangle(vertices[2],vertices[3] , vertices[7]) );
            tris.push_back( Triangle(vertices[2],vertices[7] , vertices[6]) );
        }
        if(surface[1] == true) {
            tris.push_back( Triangle(vertices[0], vertices[4] , vertices[7]) );
            tris.push_back( Triangle(vertices[0],vertices[7] , vertices[3]) );
        }
        if(surface[2] == true) {
            tris.push_back( Triangle(vertices[1],vertices[0] , vertices[4]) );
            tris.push_back( Triangle(vertices[1],vertices[4] , vertices[5]) );
        }
        if(surface[3] == true) {
            tris.push_back( Triangle(vertices[1],vertices[5] , vertices[6]) );
            tris.push_back( Triangle(vertices[1],vertices[6] , vertices[2]) );
        }
        if(surface[4] == true) {
            tris.push_back( Triangle(vertices[2],vertices[3] , vertices[0]) );
            tris.push_back( Triangle(vertices[2],vertices[0] , vertices[1]) );
        }
        if(surface[5] == true) {
            tris.push_back( Triangle(vertices[6],vertices[7] , vertices[4]) );
            tris.push_back( Triangle(vertices[6],vertices[4] , vertices[5]) );
        }
    //}
    //std::cout << " side_tris()  returning " << tris.size() << " triangles \n";
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
        //surface[n]=false;
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
