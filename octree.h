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

#ifndef OCTREE_H
#define OCTREE_H

#include <iostream>
#include <list>
#include <vector>
#include "volume.h"
class Point;
class Triangle;
class MillingCutter;

/// gargantini-code for representing octree nodes
class Ocode {
    public:
        /// create Ocode, filled with all "8"
        Ocode();
        /// copy constructor
        Ocode(const Ocode &o);
        
        /// return point corresponding to code
        Point point() const; 
        /// return corner point of cube
        Point corner(int idx);
        /// return normalized direction vector to child quadrants
        Point dir(int idx) const;
        
        /// return degree of node
        int degree() const;
        /// return scale of node
        double get_scale();
        /// set global scale 
        void set_scale(double s);
        /// return true if this node can be expanded
        bool expandable();
        /// return true if node is white
        bool isWhite(OCTVolume* vol);
        /// return true if node is grey
        bool isGrey(OCTVolume* vol);
        /// return true if this contained in o
        bool containedIn(const Ocode& o) const;
        /// return list of Ocodes for sub-octants
        std::vector<Ocode> expand();
        /// index into code
        char operator[](const int &idx);
        
        /// assignment
        Ocode &operator=(const Ocode &o);
        /// equality
        bool operator==(const Ocode &o);
        /// inequality
        bool operator!=(const Ocode &o);
        /// comparison, return true if this numerically smaller than o
        bool operator<(const Ocode& o) const;
        
        /// return numerical value
        // NOTE: this is probably a bad idea
        // depth N trees will require N-digit numbers...
        // better rewrite operator< so it does not use number()
        unsigned long number() const;
        /// set an invalid Ocode
        void null();
        /// test for the invalid Ocode set by null()
        bool isNull();

        /// get max depth of code
        int get_depth();
        /// set maximum depth of code
        void set_depth(int d);
        
        /// string repr
        friend std::ostream& operator<<(std::ostream &stream, const Ocode &o);
        /// string repr
        std::string str();
        
        // DATA
        static int depth;
        static double scale;
        static Point center;
        
        // color of node, currently not used...
        // int color;
        
        /// the code. values 0-8 are needed, so only 4-bits really required...
        std::vector<char> code;
};

/// linear octree
class LinOCT {
    public:
        LinOCT();
        
        /// list of Ocodes in this octree
        //std::vector<Ocode> clist;
        
        // alternative list-version
        std::list<Ocode> clist;
        
        /// return length of list
        int size() const;
        /// add an Ocode to the end of the list
        void append(Ocode& c);
        /// add Ocode at position idx
        void append_at(Ocode& code, int idx);
        /// initialize octree, expanding n_expand times
        void init(int n_expand);
        /// expand node at position idx
        void expand_at(int idx);
        
        void expand_at(std::list<Ocode>::iterator& it);
        
        /// delete node at position idx idx
        void delete_at(int idx);
        void delete_at(std::list<Ocode>::iterator& it);
        
        /// return true if idx is valid
        bool valid_index(int idx);
        
        /// union operation
        // TODO: compare to union created by operation()
        void sum(LinOCT& other);
        
        /// set-operations: union, intersecntion, differences: this-other and other-this
        LinOCT operation(int type, LinOCT& other);
                
        /// helper function for difference operation
        void do_diff(Ocode& H, std::vector<Ocode>& Q, std::vector<Ocode>& D);
        
         
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
        
        //OCTVolume* vol, int level, Point& center, OCTNode* parent)
        /// build octree of interior of OCTVolume
        void build(OCTVolume* vol); 
        
        /// string repr
        friend std::ostream& operator<<(std::ostream &stream, const Ocode &o);
        /// string repr
        std::string str();
        /// string_repr
        void printList();
};


#endif
