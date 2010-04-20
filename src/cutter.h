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
#include <iostream>
#include <string>
#include "point.h"
#include "triangle.h"
#include "stlsurf.h"

//class STLSurf;

namespace bp = boost::python;

///
/// \brief MillingCutter is a base-class for all milling cutters
///
class MillingCutter {
    public:
        MillingCutter();
        /// set the diameter of the cutter to d
        void setDiameter(double d);
        /// return the diameter of the cutter
        double getDiameter() const;
        /// return the radius of the cutter
        double getRadius() const;
        /// set the length of the cutter to l
        void setLength(double l);
        /// return the length of the cutter
        double getLength() const;
        
        
        /// does the cutter bounding-box, positioned at cl, overlap with the bounding-box of Triangle t?
        /// works in the xy-plane 
        bool overlaps(Point &cl, const Triangle &t) const;
        
        /// drop cutter at (cl.x, cl.y) against vertices of Triangle t.
        /// loop through each vertex p of Triangle t
        /// drop down cutter at (cl.x, cl.y) against Point p
        virtual int vertexDrop(Point &cl, CCPoint &cc, const Triangle &t) const = 0;
        /// drop cutter at (cl.x, cl.y) against facet of Triangle t
        virtual int facetDrop(Point &cl, CCPoint &cc, const Triangle &t) const = 0;
        /// drop cutter at (cl.x, cl.y) against edges of Triangle t
        virtual int edgeDrop(Point &cl, CCPoint &cc, const Triangle &t) const = 0;
        
        /// drop the MillingCutter at Point cl down along the z-axis
        /// until it makes contact with Triangle t.
        /// This function calls vertexDrop, facetDrop, and edgeDrop to do its job.
        /// Follows the template-method, or "self-delegation" design pattern.
        int dropCutter(Point &cl, CCPoint &cc, const Triangle &t) const;
        
        /// drop the MillingCutter at Point cl down along the z-axis
        /// until it makes contact with a triangle in the STLSurf s
        /// NOTE: no kd-tree optimization, this function will make 
        /// dropCutter() calls for each and every Triangle in s.
        int dropCutterSTL(Point &cl, CCPoint &cc, const STLSurf &s) const;
        
    protected:
        /// id-number count
        static int count;
        /// cutter id number
        int id;
        /// diameter of cutter
        double diameter;
        /// length of cutter
        double length;
        /// set id-number
        void setId();
};


/* required wrapper class for virtual functions in boost-python */
/// \brief a wrapper required for boost-python
class MillingCutterWrap : public MillingCutter, public bp::wrapper<MillingCutter>
{
    public:
    int vertexDrop(Point &cl, CCPoint &cc, const Triangle &t) const
    {
        return this->get_override("vertexDrop")(cl,cc,t);
    }

    int facetDrop(Point &cl, CCPoint &cc, const Triangle &t) const
    {
        return this->get_override("facetDrop")(cl,cc,t);
    }
    
    int edgeDrop(Point &cl, CCPoint &cc, const Triangle &t) const
    {
        return this->get_override("edgeDrop")(cl,cc,t);
    }    
    
};


///
/// \brief Cylindrical MillingCutter (flat-endmill)
///
/// defined by its radius
class CylCutter : public MillingCutter {
    public:
        /// create CylCutter with diameter = 1.0
        CylCutter();
        /// create CylCutter with diameter = d
        CylCutter(const double d);
        
        // dropCutter methods
        /// drop cutter at (cl.x, cl.y) against vertices of Triangle t
        int vertexDrop(Point &cl, CCPoint &cc, const Triangle &t) const;
        /// drop cutter at (cl.x, cl.y) against facet of Triangle t
        int facetDrop(Point &cl, CCPoint &cc, const Triangle &t) const;
        /// drop cutter at (cl.x, cl.y) against edges of Triangle t
        int edgeDrop(Point &cl, CCPoint &cc, const Triangle &t) const;
        
        /// text output
        friend std::ostream& operator<<(std::ostream &stream, CylCutter c);
        /// string repr
        std::string str();
        
};


/// \brief Ball or Spherical MillingCutter (ball-nose endmill)
///
/// defined by its radius
class BallCutter : public MillingCutter {
    public:
        BallCutter();
        /// create a BallCutter with diameter d and radius d/2
        BallCutter(const double d);
        int vertexDrop(Point &cl, CCPoint &cc, const Triangle &t) const;
        int facetDrop(Point &cl, CCPoint &cc, const Triangle &t) const;
        /// edge-test for BallCutter
        int edgeDrop(Point &cl, CCPoint &cc, const Triangle &t) const;
        
        /// string repr
        friend std::ostream& operator<<(std::ostream &stream, BallCutter c);
        /// string repr
        std::string str();
        
    protected:
        /// the radius of a BallCutter is by definition always half of the diameter, i.e.
        /// radius = diameter/2
        double radius;
};


/// \brief Bull-nose or Toroidal MillingCutter (filleted endmill)
///
/// defined by radius1, the cylindrical middle part of the cutter
/// and radius2, the corner radius of the fillet/torus.
class BullCutter : public MillingCutter {
    public:
        /// Create bull-cutter with default diameter and corner radius.
        BullCutter();
        /// Create bull-cutter with diamter d and corner radius r.
        BullCutter(const double d, const double r);
        
        /// bull-cutter vertex drop
        int vertexDrop(Point &cl, CCPoint &cc, const Triangle &t) const;
        /// bull-cutter facet-test 
        int facetDrop(Point &cl, CCPoint &cc, const Triangle &t) const;
        /// \todo edge-test for toroid
        int edgeDrop(Point &cl, CCPoint &cc, const Triangle &t) const;
        
        /// string repr
        friend std::ostream& operator<<(std::ostream &stream, BullCutter c);
        /// string repr
        std::string str();
    protected:
        
        /// set radius of cutter
        void setRadius();
        /// cutter radius. 
        /// radius = radius1 + radius2
        double radius;  
        /// radius of cylindrical part of cutter
        double radius1;
        /// tube radius of toroid
        double radius2;
};


/// \brief Conical MillingCutter 
///
/// cone defined by diameter and cone-angle. sharp tip. 
/// 60 degrees or 90 degrees are common
class ConeCutter : public MillingCutter {
    public:
        /// default constructor
        ConeCutter();
        /// create a ConeCutter with specified diameter and cone-angle
        ConeCutter(const double d, const double angle);
        int vertexDrop(Point &cl, CCPoint &cc, const Triangle &t) const;
        int facetDrop(Point &cl, CCPoint &cc, const Triangle &t) const;
        int edgeDrop(Point &cl, CCPoint &cc, const Triangle &t) const;
        /// string repr
        friend std::ostream& operator<<(std::ostream &stream, ConeCutter c);
        /// string repr
        std::string str();
        
    protected:
        /// the half-angle of the cone, in radians
        double angle;
        /// the height of the cone
        double height;
};













#endif

