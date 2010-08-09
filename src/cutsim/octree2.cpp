/*  $Id: $
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


namespace ocl
{

//**************** Octree ********************/

Octree::Octree(double scale, unsigned int  depth, Point& centerp) {
    root_scale = scale;
    max_depth = depth;
    root_center = centerp;
                    // parent, center, scale, depth
    root = new Octnode( 0, &root_center, root_scale, 0 );
}

// search tree and return list of leaf-nodes
boost::python::list Octree::py_get_leaf_nodes() const {
    //std::cout << " py_get_leaf_nodes() \n";
    std::vector<Octnode*> nodelist;
    //std::cout << " get_leaf_nodes() \n";
    Octree::get_leaf_nodes(root, nodelist);
    //std::cout << " got " << nodelist.size() << " nodes\n";
    boost::python::list pynodelist;
    BOOST_FOREACH(Octnode* n, nodelist) {
        pynodelist.append( *n );
    }
    return pynodelist;
}

void Octree::init(unsigned int n) {
    // subdivide n-times
    for (unsigned int m=0;m<n;++m) {
        std::vector<Octnode*> nodelist;
        Octree::get_leaf_nodes(root, nodelist);
        std::cout << " init(m="<<m<<") got "<<nodelist.size()<<" nodes to subdivide\n";
        BOOST_FOREACH( Octnode* n, nodelist) {
            n->subdivide();
        }
    }
}

// static!
void Octree::get_leaf_nodes(Octnode* current, std::vector<Octnode*>& nodelist) {
    if ( current->leaf == true ) {
        //std::cout << " current->leaf=" << current->leaf << " \n";
        nodelist.push_back( current );
    } else {
        //std::cout << " not a leaf \n";
        for ( int n=0;n<8;++n) {
            if ( current->child[n] != 0 )
                Octree::get_leaf_nodes( current->child[n], nodelist );
        }
    }
}

void Octree::build_root(OCTVolume* vol) {
    build( this->root, vol);
}

void Octree::build(Octnode* root, OCTVolume* vol) {
    std::vector<Octnode*> nodelist;
    Octree::get_leaf_nodes(root, nodelist); // get leafs of this root
    int nevals=0;
    int ndeleted=0;
    int ninside=0;
    BOOST_FOREACH( Octnode* n, nodelist) { // go through each leaf
            n->evaluate( vol );
            ++nevals;
            if ( n->outside ) {
                //std::cout << " outside node, delete\n";
                Octnode* parent = n->parent;
                if (parent)
                    parent->delete_child(n);
                ++ndeleted;
            } else if (n->inside) {
                //std::cout << " inside node, do nothing\n";
                ++ninside;
            } else {
                // std::cout << " intermediate node, subdivide\n";
                if ( root->depth < (this->max_depth-1) ) {
                    n->subdivide();
                    for(int m=0;m<8;++m) {
                        Octree::build( n->child[m], vol); // build child
                    }
                }
            }
    }
    
    //std::cout << " build() made "<<nevals<<" vol->evaluate() calls\n";
    //std::cout << " build() "<<ndeleted<<" nodes were deleted\n";
    //std::cout << " build() "<<ninside<<" nodes inside\n";
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
    o << "  " << nodelist.size() << " leaf-nodes";
    int m=0;
    BOOST_FOREACH( int count, nodelevel) {
        o << "depth="<<m <<" has " << count << " nodes\n";
        ++m;
    }
    return o.str();
}

//**************** Octnode ********************/

Point Octnode::direction[8] = {
                    Point( 1, 1,-1),
                    Point(-1, 1,-1),
                    Point(-1,-1,-1),
                    Point( 1,-1,-1),
                    Point( 1, 1, 1),
                    Point(-1, 1, 1),
                    Point(-1,-1, 1),
                    Point( 1,-1, 1)
                    };

Octnode::Octnode(Octnode* nodeparent, Point* centerp, double nodescale, unsigned int nodedepth) {
    parent = nodeparent;
    center = centerp;
    scale = nodescale;
    depth = nodedepth;
    leaf = true;
    setvertices();
}

void Octnode::delete_child(Octnode* c) {
    for( int n=0;n<8;++n ) {
        if ( this->child[n] == c ) { // FIXMEEE
            delete this->child[n];
            this->child[n] = 0;
        }
    }
}

/// create 8 children of this node
void Octnode::subdivide() {
    if (this->leaf) {
        for( int n=0;n<8;++n ) {
            //Point* childc = ;
            //std::cout << " childcenter("<<n<<") is " << childc << "\n";
            this->child[n] = new Octnode( this, this->childcenter(n) , scale/2.0 , depth+1 );
            // inherit the surface property here...
        }
        this->leaf = false;
    }
}

void Octnode::evaluate(OCTVolume* vol) {
    //bool outside=true;
    //bool inside=true;
    outside = true;
    inside = true;
    for ( int n=0;n<8;++n) {
        f[n] = vol->dist( *(vertex[n]) );
        if ( f[n] <= 0.0 ) // if one vertex is inside
            outside = false; // then it's not an outside-node
        else { // if one vertex is outside
            assert( f[n] > 0.0 );
            inside = false; // then it's not an inside node anymore
        }
    }
}



/// return centerpoint of child n
Point* Octnode::childcenter(int n) {
    return  new Point(*center + (0.5*scale * direction[n]));
}

/// set the vertex positions
void Octnode::setvertices() {
    for ( int n=0;n<8;++n) {
        vertex[n] = new Point(*center + scale*direction[n] ) ;
    }
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
