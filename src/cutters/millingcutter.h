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

#include "stlsurf.h"
#include "fiber.h"
#include "point.h"
#include "clpoint.h"
#include "ccpoint.h"

namespace ocl
{

class Triangle;
class STLSurf;

typedef std::pair< double, double > CC_CLZ_Pair;

///
/// \brief MillingCutter is a base-class for all milling cutters
///
class MillingCutter {
    public:
        /// default constructor
        MillingCutter() {}
        virtual ~MillingCutter() {}
        
        /// return the diameter of the cutter
        inline double getDiameter() const { return diameter; }
        /// return the radius of the cutter
        inline double getRadius() const { return radius; }
        /// return the length of the cutter
        inline double getLength() const { return length; }
        
        /// return a MillingCutter which is larger than *this by d
        virtual MillingCutter* offsetCutter(double d) const;
        
        /// does the cutter bounding-box, positioned at cl, overlap with the bounding-box of Triangle t?
        /// works in the xy-plane 
        bool overlaps(Point &cl, const Triangle &t) const;
        
        /// drop cutter at (cl.x, cl.y) against the three vertices of Triangle t.
        /// calls this->height(r) on the subclass of MillingCutter we are using.
        virtual bool vertexDrop(CLPoint &cl, const Triangle &t) const;
        
        /// drop cutter at (cl.x, cl.y) against facet of Triangle t
        /// calls xy_normal_length(), normal_length(), and center_height() on the subclass
        virtual bool facetDrop(CLPoint &cl, const Triangle &t) const;
        
        /// drop cutter at (cl.x, cl.y) against the three edges of input Triangle t.
        /// calls the sub-class MillingCutter::singleEdgeDrop on each edge
        virtual bool edgeDrop(CLPoint& cl, const Triangle &t) const;
        
        /// drop the MillingCutter at Point cl down along the z-axis
        /// until it makes contact with Triangle t.
        /// This function calls vertexDrop, facetDrop, and edgeDrop to do its job.
        /// Follows the template-method, or "self-delegation" design pattern.
        bool dropCutter(CLPoint &cl, const Triangle &t) const;

        /// drop the MillingCutter at Point cl down along the z-axis
        /// until it makes contact with a triangle in the STLSurf s
        /// NOTE: no kd-tree optimization, this function will make 
        /// dropCutter() calls for each and every Triangle in s.
        /// NOTE: should not really be used for real work, demo/debug only
        bool dropCutterSTL(CLPoint &cl, const STLSurf &s) const;
        
        /// push the cutter along Fiber f into contact with the vertices of Triangle t, updating Interval i
        /// with the interfering interval.
        virtual bool vertexPush(const Fiber& f, Interval& i, const Triangle& t) const;
        /// push cutter along Fiber f into contact with facet of Triangle t, and update Interval i
        virtual bool facetPush(const Fiber& f, Interval& i, const Triangle& t) const;
        /// push cutter along Fiber f into contact with edges of Triangle t, update Interval i
        bool edgePush(const Fiber& f, Interval& i, const Triangle& t) const;
        
        /// return a string representation of the MillingCutter
        virtual std::string str() const {return "MillingCutter (all derived classes should override this)";}
        
    protected:
        /// push cutter along fiber against a single edge p1-p2
        virtual bool singleEdgePush(const Fiber& f, Interval& i,  const Point& p1, const Point& p2) const {return false;}
        
        /// push-cutter cylindrical shaft case
        bool shaftEdgePush(const Fiber& f, Interval& i,  const Point& p1, const Point& p2) const;
        
        
        /// drop cutter against edge p1-p2 at xy-distance d from cl
        virtual bool singleEdgeDrop(CLPoint& cl, const Point& p1, const Point& p2, double d) const;
        
        /// 'canonical' position with cl=(0,0,cl.z) and edge u1-u2 along x-axis 
        /// returns x-coordinate of cc-point and cl.z
        virtual CC_CLZ_Pair singleEdgeContact(const Point& u1, const Point& u2) const;
        
        /// return the height of the cutter at radius r.
        /// should be redefined by a subclass.
        virtual double height(double r) const {assert(0); return -1;}
        
        /// return the width of the cutter at height h.
        /// should be redefined by a subclass.
        virtual double width(double h) const {assert(0); return -1;}
        
        /// xy_normal lenght that locates the cutter center relative to a
        /// cc-point on a facet.
        double xy_normal_length;
        
        /// normal lenght that locates the cutter center relative to a
        /// cc-point on a facet.
        double normal_length;
        
        /// height of cutter center along z-axis
        double center_height;
        
        /// diameter of cutter
        double diameter;
        
        /// radius of cutter
        double radius;
        
        /// length of cutter
        double length;
};

} // end namespace
#endif
// end file millingcutter.h
