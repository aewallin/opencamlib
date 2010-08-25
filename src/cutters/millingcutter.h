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

#ifndef MILLING_CUTTER_H
#define MILLING_CUTTER_H

#include <iostream>
#include <string>
#include <vector>

#include <boost/python.hpp>

#include "stlsurf.h"
#include "fiber.h"
#include "point.h"
#include "clpoint.h"
#include "ccpoint.h"

namespace ocl
{

class Triangle;
class STLSurf;

///
/// \brief MillingCutter is a base-class for all milling cutters
///
class MillingCutter {
    public:
        /// default constructor
        MillingCutter();
        virtual ~MillingCutter() {};
        
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
        
        /// return a MillingCutter which is larger than *this by d
        virtual MillingCutter* offsetCutter(const double d) const;
        
        /// does the cutter bounding-box, positioned at cl, overlap with the bounding-box of Triangle t?
        /// works in the xy-plane 
        bool overlaps(Point &cl, const Triangle &t) const;
        
        /// drop cutter at (cl.x, cl.y) against the three vertices of Triangle t.
        /// calls this->height(r) on the subclass of MillingCutter we are using.
        virtual int vertexDrop(CLPoint &cl, const Triangle &t) const;
        
        /// drop cutter at (cl.x, cl.y) against facet of Triangle t
        /// calls xy_normal_length(), normal_length(), and center_height() on the subclass
        virtual int facetDrop(CLPoint &cl, const Triangle &t) const;
        
        /// drop cutter at (cl.x, cl.y) against the three edges of Triangle t
        /// needs to be defined by a subclass
        virtual int edgeDrop(CLPoint &cl, const Triangle &t) const {return -1;};
        
        /// drop the MillingCutter at Point cl down along the z-axis
        /// until it makes contact with Triangle t.
        /// This function calls vertexDrop, facetDrop, and edgeDrop to do its job.
        /// Follows the template-method, or "self-delegation" design pattern.
        int dropCutter(CLPoint &cl, const Triangle &t) const;

        /// drop the MillingCutter at Point cl down along the z-axis
        /// until it makes contact with a triangle in the STLSurf s
        /// NOTE: no kd-tree optimization, this function will make 
        /// dropCutter() calls for each and every Triangle in s.
        // should not really be used for real work, demo/debug only
        int dropCutterSTL(CLPoint &cl, const STLSurf &s) const;
        
        // pushCutter methods
        virtual bool vertexPush(const Fiber& f, Interval& i, const Triangle& t) const;
        virtual bool facetPush(const Fiber& f, Interval& i, const Triangle& t) const;
        virtual bool edgePush(const Fiber& f, Interval& i, const Triangle& t) const {assert(0);return false;};
        
        virtual std::string str() const {return "MillingCutter (all derived classes should override this)";};
        
    protected:
    
        /// return the height of the cutter at radius r.
        /// should be redefined by a subclass.
        virtual double height(const double r) const {assert(0); return -1;};
        /// return the width of the cutter at height h.
        /// should be redefined by a subclass.
        virtual double width(const double h) const {assert(0); return -1;};
        /// return xy_normal lenghts that locate the cutter center relative to a
        /// cc-point on a facet.
        virtual double xy_normal_length() const {assert(0); return -1;};
        virtual double normal_length() const {assert(0); return -1;};
        virtual double center_height() const {assert(0); return -1;};
        
        /// diameter of cutter
        double diameter;
        /// radius of cutter
        double radius;
        /// length of cutter
        double length;
};


/* required wrapper class for virtual functions in boost-python */
/// \brief a wrapper required for boost-python
// see documentation:
// http://www.boost.org/doc/libs/1_43_0/libs/python/doc/tutorial/doc/html/python/exposing.html#python.inheritance

class MillingCutterWrap : public MillingCutter, public boost::python::wrapper<MillingCutter>
{
    public:
        // vertex
        int vertexDrop(CLPoint &cl, const Triangle &t) const
        {
            if ( boost::python::override ovr_vertexDrop = this->get_override("vertexDrop"))
                return ovr_vertexDrop(cl, t);
            return MillingCutter::vertexDrop(cl, t);
        };
        int default_vertexDrop(CLPoint &cl, const Triangle &t) const{
            return this->MillingCutter::vertexDrop(cl,t);
        };
        
        // facet
        int facetDrop(CLPoint &cl, const Triangle &t) const
        {
            if ( boost::python::override ovr_facetDrop = this->get_override("facetDrop"))
                return ovr_facetDrop(cl, t);
            return MillingCutter::facetDrop(cl, t);
        };
        int default_facetDrop(CLPoint &cl, const Triangle &t) const{
            return this->MillingCutter::facetDrop(cl,t);
        };
        
        
        // edge
        int edgeDrop(CLPoint &cl, const Triangle &t) const
        {   
            if ( boost::python::override ovr_edgeDrop = this->get_override("edgeDrop"))
                return ovr_edgeDrop(cl, t);
            return MillingCutter::edgeDrop(cl, t);
        };    
        int default_edgeDrop(CLPoint &cl, const Triangle &t) const{
            return this->MillingCutter::edgeDrop(cl,t);
        };
        
        // offset cutter
        MillingCutter* offsetCutter(const double d) const
        {
            if ( boost::python::override ovr_offsetCutter = this->get_override("offsetCutter") )
                return ovr_offsetCutter(d);
            return MillingCutter::offsetCutter(d);
        };   
        MillingCutter* default_offsetCutter(const double d) const {
            return this->MillingCutter::offsetCutter(d);
        };
        
        // string repr
        std::string str() const
        {
            if ( boost::python::override ovr_str = this->get_override("str")) {
                return ovr_str();
            }
            return MillingCutter::str();
        }; 
        std::string default_str() const {
            return this->MillingCutter::str();
        };
};


} // end namespace
#endif
// end file millingcutter.h
