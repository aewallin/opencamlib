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

#ifndef OCTREE_H
#define OCTREE_H

#include <iostream>
#include <list>

#include "ocode.h"
#include "volume.h"

namespace ocl
{

/// linear octree
class LinOCT {
    public:
        /// create an empty octree.
        /// must call init() before using it!
        LinOCT();
        
        /// list of Ocodes in this octree
        std::list<Ocode> clist;
        
        /// return length of list
        int size() const;
        /// add an Ocode to the end of the list
        void append(Ocode& c);
        /// initialize octree, calling expand() n_expand times
        void init(int n_expand=0);
        /// expand node at iterator it
        void expand_at(std::list<Ocode>::iterator& it);
        
        /// delete node at iterator it
        void delete_at(std::list<Ocode>::iterator& it);
               
        /// union operation
        // TODO: compare to union created by operation()
        void sum(LinOCT& other);
        
        /// set-operations: union, intersecntion, differences: this-other and other-this
        LinOCT operation(int type, LinOCT& other);
                
        /// helper function for difference operation
        void do_diff(Ocode& H, std::list<Ocode>& Q, std::vector<Ocode>& D);
        
        /// remove other from this
        void diff(LinOCT& other);
         
        /// sort the list
        void sort();
        
        /// condense list
        void condense();
        
        /// starting at idx, can the following 8 nodes be collapsed into one
        bool can_collapse_at(int idx);
        
        /// return all nodes as a list to python
        boost::python::list get_nodes();
        /// return list of triangles to python
        boost::python::list get_triangles();
        /// build octree of interior of OCTVolume
        void build(OCTVolume* vol); 
        
        /// string repr
        friend std::ostream& operator<<(std::ostream &stream, const Ocode &o);
        /// string repr
        std::string str() const;
        /// string repr: print out list of all Ocodes in tree
        void printList();
};

} // end namespace
#endif
// end file octree.h
