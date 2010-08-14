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

#ifndef OCTREE2_H
#define OCTREE2_H

#include <iostream>
#include <list>

#include "point.h"
#include "volume.h"
#include "triangle.h"
#include "bbox.h"

namespace ocl
{




/// octree node
class Octnode {
    public:
        Octnode(){};
        Octnode(Octnode* parent, unsigned int idx, double nodescale, unsigned int nodedepth);
        virtual ~Octnode();
        void subdivide(); // create children
        Point* childcenter(int n); // return position of child centerpoint
        void setvertices(); // set vertices[]
        void evaluate(OCTVolume* vol);

        void delete_child(unsigned int index);
        std::vector<Triangle> mc_triangles();

        Point interpolate(int idx1, int idx2);
        
        // python interface
        boost::python::list py_get_vertices() const;
        Point py_get_center() const;
        
    // DATA
        /// pointers to child nodes
        std::vector<Octnode*> child;
        /// pointer to parent node
        Octnode* parent;
        /// a leaf ndoe?
        bool leaf; 
        /// The eight corners of this node
        std::vector<Point*> vertex; 
        /// value of implicit function at vertex
        std::vector<double> f; 
        bool outside;
        bool inside;
        bool surface[6]; // flag for surface triangles FIXME!
        Point* center; // the centerpoint of this node
        unsigned int depth; // depth of node
        unsigned int idx; // index of node
        double scale; // distance from center to vertices
        bool evaluated;
        Bbox bb;
        
        std::vector<Triangle> mc_tris;
        bool mc_tris_valid;
        
        /// the direction to the vertices, from the center 
        static Point direction[8];
        static const unsigned int edgeTable[256];
        static const int triTable[256][16];
        
        
        /// string repr
        friend std::ostream& operator<<(std::ostream &stream, const Octnode &o);
        /// string repr
        std::string str() const;
};

class Octree {
    public:
        Octree() {};
        Octree(double root_scale, unsigned int max_depth, Point& centerp);
        std::string str() const;
        void diff_negative_root(OCTVolume* vol);
        void diff_negative(Octnode* current, OCTVolume* vol);
        void prune_inside_root();
        void prune_inside( Octnode* current );
        
        void get_leaf_nodes(Octnode* current, std::vector<Octnode*>& nodelist) const;
        void get_all_nodes(Octnode* current, std::vector<Octnode*>& nodelist) const;
        std::vector<Triangle> mc();
        void init(unsigned int n);
        
    // python interface
        boost::python::list py_get_leaf_nodes() const;
        //boost::python::list py_get_surface_nodes() const;
        boost::python::list py_mc_triangles(); 
    // DATA
        double root_scale;
        unsigned int max_depth;
        //Point root_center;
        /// root node of tree
        Octnode* root;

};

} // end namespace
#endif
// end file octree2.h
