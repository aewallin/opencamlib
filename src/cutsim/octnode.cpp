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

#include <boost/foreach.hpp>

#include "point.h"
#include "triangle.h"
#include "numeric.h"
#include "octnode.h"
#include "volume.h"
#include "mc_tables.h"

namespace ocl
{

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
    
    for ( int n=0;n<8;++n) {
        vertex[n] = new Point(*center + scale*direction[n] ) ;
        f[n] = 1e6;
        //surface[n]=false;
    }
    bb.clear();
    bb.addPoint( *vertex[2] ); // vertex[2] has the minimum x,y,z coordinates
    bb.addPoint( *vertex[4] ); // vertex[4] has the max x,y,z
    
    
    mc_tris_valid = false;
    evaluated = false;
    childcount = 0;
}

// call delete on children, vertices, and center
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


void Octnode::set_surfaces() {
    assert( parent );
    surface = parent->surface; // copy surface flags from parent
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

// create the 8 children of this node
void Octnode::subdivide() {
    if (this->childcount==0) {
        for( int n=0;n<8;++n ) {
            this->child[n] = new Octnode( this, n , scale/2.0 , depth+1 ); // parent,  idx, scale,   depth
            ++childcount;
            // inherit the surface property here
            // optimization: inherit one f[n] from the corner?
        }
    } else {
        std::cout << " DON'T subdivide a non-leaf node \n";
        assert(0); 
    }
}

// evaluate vol->dist() at all the vertices and store in f[]
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

// interpolate f-value between vertices idx1 and idx2
Point Octnode::interpolate(int idx1, int idx2) {
    // p = p1 - v1 (p2-p1)/(v2-v1)
    assert( !isZero_tol( f[idx2]-f[idx1]  ) );
    return *vertex[idx1] - f[idx1]*( *vertex[idx2]-(*vertex[idx1]) ) * 
                                    (1.0/(f[idx2]-f[idx1]));
}

// use marching-cubes and return a list of triangles for this node
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
    vertices[0] = *vertex[0];
    vertices[1] = *vertex[1];
    vertices[2] = *vertex[2];
    vertices[3] = *vertex[3];
    vertices[4] = *vertex[4];
    vertices[5] = *vertex[5];
    vertices[6] = *vertex[6];
    vertices[7] = *vertex[7];
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
    for (unsigned int i=0; triTable[edgeTableIndex][i] != -1 ; i+=3 ) {
        tris.push_back( Triangle( vertices[ triTable[edgeTableIndex][i  ] ],
                                  vertices[ triTable[edgeTableIndex][i+1] ], 
                                  vertices[ triTable[edgeTableIndex][i+2] ]
                                 ) );
    }
    
    std::vector<Triangle> crack_tris;
    crack_tris = crack_triangles(vertices);
    
    for( unsigned int i = 0; i < crack_tris.size(); ++i)
            tris.push_back( crack_tris[i] );
                
    this->mc_tris = tris;
    this->mc_tris_valid = true;
    return tris;
}

// marching-cubes doesn't produce triangles between an original unmodified octree
// and a cut region. these extra triangles should cover the cracks that result.
// TODO: only seems to work for the top-surface of the XY-plane right now.
std::vector<Triangle> Octnode::crack_triangles(std::vector<Point>& vertices) {
    assert( childcount == 0); // don't call this on non-leafs!
    std::vector<Triangle> tris;
    
    if ( this->outside || this->inside || !evaluated) {
        return tris; // outside nodes do not produce triangles
    } 
    if(!surface[0] && !surface[1] && !surface[2] && !surface[3] && !surface[4] && !surface[5])
        return tris; // return early if no surface flags set

    //init as surface[5] = true
    int nV1 = 4; 
    int nV2 = 5;
    int nV3 = 6;
    int nV4 = 7;
    // surf     vertices  vertices
    // 0:       2,3,7     2,6,7
    // 1:       0,4,7     0,3,7
    // 2:       0,1,4     1,4,5
    // 3:       1,5,6     1,2,6
    // 4:       0,2,3     0,1,2
    // 5:       4,6,7     4,5,6
    if(surface[0]) { //2376 
            nV1 = 2; 
            nV2 = 3;
            nV3 = 7;
            nV4 = 6;
    } else if(surface[1]) { //0473
            nV1 = 0; 
            nV2 = 4;
            nV3 = 7;
            nV4 = 3;
    } else if(surface[2]) {//0451
            nV1 = 0; 
            nV2 = 4;
            nV3 = 5;
            nV4 = 1;
    } else if(surface[3]) { //1562
            nV1 = 1; 
            nV2 = 5;
            nV3 = 6;
            nV4 = 2;
    } else if(surface[4]) { //1562
            nV1 = 0; 
            nV2 = 1;
            nV3 = 2;
            nV4 = 3;
    }
    double df[8];
    for(int i = 0; i < 8; i++)
            df[i] = f[i];
    
    if(f[nV4] <0 && f[nV1]>0 && f[nV2]>0 && f[nV3]>0 ) { //only nV4 of surface in tool
        tris.push_back( Triangle(*vertex[ nV2 ],*vertex[ nV3 ],vertices[nV3]));
        tris.push_back( Triangle(*vertex[ nV2 ],vertices[ nV3 ],vertices[nV4]));
        tris.push_back( Triangle(*vertex[ nV2 ],vertices[ nV4 ],*vertex[nV1]));
    } else if(f[nV3] <0 && f[nV1]>0 && f[nV2]>0 && f[nV4]>0) { //only nV3 of surface in tool
        tris.push_back( Triangle(*vertex[ nV1 ],*vertex[ nV2 ],vertices[nV2]));
        tris.push_back( Triangle(*vertex[ nV1 ],vertices[ nV2 ],vertices[nV3]));
        tris.push_back( Triangle(*vertex[ nV1 ],vertices[ nV3 ],*vertex[nV4]));
    } else if(f[nV2] <0 && f[nV1]>0 && f[nV3]>0 && f[nV4]>0) {//only nV2 of surface in tool
        tris.push_back( Triangle(*vertex[ nV4 ],*vertex[ nV1 ],vertices[nV1]));
        tris.push_back( Triangle(*vertex[ nV4 ],vertices[ nV1 ],vertices[nV2]));
        tris.push_back( Triangle(*vertex[ nV4 ],vertices[ nV2 ],*vertex[nV3]));
    } else if(f[nV1] <0 && f[nV2]>0 && f[nV3]>0 && f[nV4]>0) { //only nV1 of surface in tool    
        tris.push_back( Triangle(*vertex[ nV3 ],*vertex[ nV4 ],vertices[nV4]));
        tris.push_back( Triangle(*vertex[ nV3 ],vertices[ nV4 ],vertices[nV1]));
        tris.push_back( Triangle(*vertex[ nV3 ],vertices[ nV1 ],*vertex[nV2]));
    } else if(f[nV4]>0 && f[nV1] <= 0 && f[nV2] <= 0 && f[nV3] <= 0) { //only 7 out of tool //only nV4 of surface out of tool    
        tris.push_back( Triangle(vertices[nV4 ],vertices[ nV3 ],*vertex[nV4]));
    } else if(f[nV3]>0 && f[nV1] <= 0 && f[nV2] <= 0 && f[nV4] <= 0) { //only 6 out of tool //only nV3 of surface out of tool
        tris.push_back( Triangle(vertices[nV2 ],*vertex[nV3],vertices[nV3]));
    } else if(f[nV2]>0 && f[nV1] <= 0 && f[nV3] <= 0 && f[nV4] <= 0) { //only nV2 out of tool
        tris.push_back( Triangle(vertices[nV1 ], vertices[ nV2 ],*vertex[nV2]));
    } else if(f[nV1]>0 && f[nV2] <= 0 && f[nV3] <= 0 && f[nV4] <= 0) { //only 4 out of tool//only nV1 of surface out of tool
        tris.push_back( Triangle(vertices[nV1 ],vertices[ nV4 ],*vertex[nV1]));
    } else if(f[nV1] < 0 && f[nV4] < 0 && f[nV2] >= 0 && f[nV3] >= 0 ) { //4 and 7 in tool and 5,6 out //only nV1,nV4 of surface in tool
        tris.push_back( Triangle(vertices[nV1 ],vertices[ nV3 ],*vertex[nV3]) );
        tris.push_back( Triangle(vertices[nV1 ],*vertex[ nV3 ], *vertex[nV2]) );
    } else if(f[nV2] < 0 && f[nV3] < 0 && f[nV1] >= 0 && f[nV4] >= 0 ) { //5 and 6 in tool and 4,7 out//only nV2,nV3 of surface in tool
        tris.push_back( Triangle(*vertex[nV4],vertices[nV1],vertices[ nV3 ]));
        tris.push_back( Triangle(*vertex[nV4],*vertex[nV1],vertices[ nV1 ]));
    } else if(f[nV1] < 0 && f[nV2] < 0 && f[nV3] >= 0 && f[nV4] >= 0 ) { //4 and 5 in tool and 6,7 out //only nV1,nV2 of surface in tool
        tris.push_back( Triangle(vertices[nV4],vertices[nV2],*vertex[ nV3 ]));
        tris.push_back( Triangle(vertices[nV4],*vertex[nV3],*vertex[ nV4 ]));
    } else if(f[nV3] < 0 && f[nV4] < 0 && f[nV1] >= 0 && f[nV2] >= 0 ) { //6 and 7 in tool and 4,5 out //only nV3,nV4 of surface in tool
        tris.push_back( Triangle(vertices[nV2],vertices[nV4],*vertex[ nV1 ]));
        tris.push_back( Triangle(vertices[nV2],*vertex[nV1],*vertex[ nV2 ]));
    }
    
    return tris;
}

// these are the side-triangles of an un-cut octree (e.g. initial stock shape)
std::vector<Triangle> Octnode::side_triangles() {
    assert( this->childcount == 0 ); // this is a leaf-node
    std::vector<Triangle> tris;
    if(!surface[0] && !surface[1] && !surface[2] && !surface[3] && !surface[4] && !surface[5]) // no surface faces
        return tris; // return early
    if ( !this->inside) {
        if(!outside && evaluated) {
            return tris; // return early, why?
        }
        if(surface[0] == true) {
            tris.push_back( Triangle( *vertex[2], *vertex[3] , *vertex[7]) );
            tris.push_back( Triangle( *vertex[2], *vertex[7] , *vertex[6]) );
        }
        if(surface[1] == true) {
            tris.push_back( Triangle( *vertex[0], *vertex[4] , *vertex[7]) );
            tris.push_back( Triangle( *vertex[0], *vertex[7] , *vertex[3]) );
        }
        if(surface[2] == true) {
            tris.push_back( Triangle( *vertex[1], *vertex[0] , *vertex[4]) );
            tris.push_back( Triangle( *vertex[1], *vertex[4] , *vertex[5]) );
        }
        if(surface[3] == true) {
            tris.push_back( Triangle( *vertex[1], *vertex[5] , *vertex[6]) );
            tris.push_back( Triangle( *vertex[1], *vertex[6] , *vertex[2]) );
        }
        if(surface[4] == true) {
            tris.push_back( Triangle( *vertex[2], *vertex[3] , *vertex[0]) );
            tris.push_back( Triangle( *vertex[2], *vertex[0] , *vertex[1]) );
        }
        if(surface[5] == true) {
            tris.push_back( Triangle( *vertex[6], *vertex[7] , *vertex[4]) );
            tris.push_back( Triangle( *vertex[6], *vertex[4] , *vertex[5]) );
        }
    }
    return tris;
}

// return centerpoint of child with index n
Point* Octnode::childcenter(int n) {
    return  new Point(*center + (0.5*scale * direction[n]));
}


// print out the boolean array of surface-flags
void Octnode::print_surfaces() {
    for (int n=0;n<6;++n)
        std::cout << surface[n];
    std::cout << "\n";
}


// string repr
std::ostream& operator<<(std::ostream &stream, const Octnode &n) {
    stream << " Octnode: N=";     
    return stream;
}

// string repr
std::string Octnode::str() const {
    std::ostringstream o;
    o << *this;
    return o.str();
}

} // end namespace
// end of file octnode.cpp
