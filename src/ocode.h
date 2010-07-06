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

#ifndef OCODE_H
#define OCODE_H

#include <iostream>
#include <list>
#include <vector>

// #include "volume.h"

namespace ocl
{
    
class Point;
class Triangle;
class MillingCutter;
class OCTVolume;

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
        /// a node is white if none of the corners and centre are inside the volume
        bool isWhite(OCTVolume* vol);
        
        /// return true if node is grey
        bool isGrey(OCTVolume* vol);
        
        /// return true if this contained in o
        bool containedIn(const Ocode& o) const;
        /// return list of Ocodes for sub-octants
        std::list<Ocode> expand();
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
        /// length of the Ocode, or maximum depth of octree
        static int depth;
        /// maximum extent of the octree
        static double scale;
        /// center point of octree
        static Point center;
        
        // color of node, currently not used...
        // int color;
        
        /// the code. values 0-8 are needed, so only 4-bits really required...
        std::vector<char> code;
};


} // end namespace
#endif
// end file ocode.h
