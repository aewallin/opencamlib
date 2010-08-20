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
        void evaluate(const OCTVolume* vol);

        std::vector<Triangle> mc_triangles();
        std::vector<Triangle> side_triangles();
        
        Point interpolate(int idx1, int idx2);
        
        // python interface
        boost::python::list py_get_vertices() const;
        Point py_get_center() const;
        
    // DATA
        /// pointers to child nodes
        std::vector<Octnode*> child;
        unsigned int childcount;
        /// pointer to parent node
        Octnode* parent;
        /// a leaf node?
        //bool leaf; 
        /// The eight corners of this node
        std::vector<Point*> vertex; 
        /// value of implicit function at vertex
        std::vector<double> f; 
        bool outside;
        bool inside;
        std::vector<bool> surface; // flag for surface triangles FIXME!
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
        /// Marching-Cubes edge table
        static const unsigned int edgeTable[256];
        /// Marching-Cubes triangle table
        static const int triTable[256][16];

        /// string repr
        friend std::ostream& operator<<(std::ostream &stream, const Octnode &o);
        /// string repr
        std::string str() const;
        
    private:
        void print_surfaces(); 
        void set_surfaces();
};

class Octree {
    public:
        Octree() { assert(0); };
        virtual ~Octree();
        Octree(double root_scale, unsigned int max_depth, Point& centerp);
        void diff_negative_root(const OCTVolume* vol);
        void get_leaf_nodes(Octnode* current, std::vector<Octnode*>& nodelist) const;
        void get_all_nodes(Octnode* current, std::vector<Octnode*>& nodelist) const;
        std::vector<Triangle> mc();
        std::vector<Triangle> side_triangles();
        void init(const unsigned int n);
        unsigned int get_max_depth() const;
        double get_root_scale() const;
        double leaf_scale() const;
        std::string str() const;
        
    // python interface
        boost::python::list py_get_leaf_nodes() const;
        boost::python::list py_mc_triangles(); 
        boost::python::list py_s_triangles(); 
        
    
    private:
        void diff_negative(Octnode* current, const OCTVolume* vol);
    // DATA
        double root_scale;
        unsigned int max_depth;
        Octnode* root;
};

} // end namespace
#endif
// end file octree2.h
