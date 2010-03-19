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



#ifndef CUTTER_H
#define CUTTER_H
#include <boost/foreach.hpp>
#include <boost/python.hpp>
//#include <boost/python/module.hpp>
//#include <boost/python/class.hpp>
//#include <boost/python/wrapper.hpp>
//#include <boost/python/call.hpp>



#include <iostream>
#include <string>
#include "point.h"
#include "triangle.h"
#include "stlsurf.h"
namespace bp = boost::python;

///
/// \brief MillingCutter is a base-class for all milling cutters
///
      
class MillingCutter {
    public:
        MillingCutter();
        
        void setDiameter(double d);
        double getDiameter();
        double getRadius();
        void setLength(double l);
        double getLength();
        void setId();
        bool overlaps(Point &cl, Triangle &t);
        
        // drop-cutter methods
        /// \brief drop cutter at (cl.x, cl.y) against vertices of Triangle t.
        /// returns the cc point
        virtual int vertexDrop(Point &cl, CCPoint &cc, const Triangle &t) = 0;
        /// drop cutter at (cl.x, cl.y) against facet of Triangle t
        virtual int facetDrop(Point &cl, CCPoint &cc, const Triangle &t) = 0;
        /// drop cutter at (cl.x, cl.y) against edges of Triangle t
        virtual int edgeDrop(Point &cl, CCPoint &cc, const Triangle &t) = 0;
        
        int dropCutter(Point &cl, CCPoint &cc, const Triangle &t);
        int dropCutterSTL(Point &cl, CCPoint &cc, const STLSurf &s);
        
    protected:
        static int count;
        /// cutter id number
        int id;
        /// diameter of cutter
        double diameter;
        /// length of cutter
        double length;
};


///
/// \brief Cylindrical milling cutter (flat-endmill)
///
class CylCutter : public MillingCutter {
    public:
        /// create CylCutter with radius = 1.0
        CylCutter();
        /// create CylCutter with diameter = d
        CylCutter(const double d);
        
        // dropCutter methods
        /// drop cutter at (cl.x, cl.y) against vertices of Triangle t
        int vertexDrop(Point &cl, CCPoint &cc, const Triangle &t);
        /// drop cutter at (cl.x, cl.y) against facet of Triangle t
        int facetDrop(Point &cl, CCPoint &cc, const Triangle &t);
        /// drop cutter at (cl.x, cl.y) against edges of Triangle t
        int edgeDrop(Point &cl, CCPoint &cc, const Triangle &t);
        
        
        
        // text output
        friend std::ostream& operator<<(std::ostream &stream, CylCutter c);
        std::string str();
        
};

///
/// \brief Ball or Spherical milling cutter (ball-nose endmill)
///
class BallCutter : public MillingCutter {
    public:
        BallCutter();
        int vertexDrop(Point &cl, CCPoint &cc, const Triangle &t);
        int facetDrop(Point &cl, CCPoint &cc, const Triangle &t);
        int edgeDrop(Point &cl, CCPoint &cc, const Triangle &t);
};

/* required wrapper class for virtual functions in boost-python */


class MillingCutterWrap : public MillingCutter, public bp::wrapper<MillingCutter>
{
    public:
    int vertexDrop(Point &cl, CCPoint &cc, const Triangle &t)
    {
        return this->get_override("vertexDrop")(cl,cc,t);
    }

    int facetDrop(Point &cl, CCPoint &cc, const Triangle &t)
    {
        return this->get_override("facetDrop")(cl,cc,t);
    }
    
    int edgeDrop(Point &cl, CCPoint &cc, const Triangle &t)
    {
        return this->get_override("edgeDrop")(cl,cc,t);
    }    
    
};



#endif

