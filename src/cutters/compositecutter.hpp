/*  $Id$
 * 
 *  Copyright (c) 2010-2011 Anders Wallin (anders.e.e.wallin "at" gmail.com).
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

#ifndef COMPOSITE_CUTTER_H
#define COMPOSITE_CUTTER_H

#include <iostream>
#include <string>
#include <vector>

#include "millingcutter.hpp"

namespace ocl
{

/// \brief a CompositeCutter is composed one or more MillingCutters
/// the cutters are stored in a vector *cutter* and their axial offsets
/// from eachother in *zoffset*. The different cutters apply in different
/// radial regions. cutter[0] from r=0 to r=radius[0] after that 
/// cutter[1] from r=radius[0] to r=radius[1] and so on. 
class CompositeCutter : public MillingCutter {
    public:
        /// create an empty CompositeCutter
        CompositeCutter();
        /// add a MillingCutter to this CompositeCutter
        /// the cutter is valid from the previous radius out to the given radius
        /// and its axial offset is given by zoffset
        void addCutter(MillingCutter& c, double radius, double height, double zoff);
        

        
        MillingCutter* offsetCutter(double d) const;
        
        /// CompositeCutter can not use the base-class facetDrop, instead we here
        /// call facetDrop() on each cutter in turn, and pick the valid CC/CL point 
        /// as the result for the CompositeCutter
        bool facetDrop(CLPoint &cl, const Triangle &t) const;
        /// call edgeDrop on each cutter and pick the correct (highest valid CL-point) result
        bool edgeDrop(CLPoint &cl, const Triangle &t) const;
        
        std::string str() const;
    protected:   
        
        bool vertexPush(const Fiber& f, Interval& i, const Triangle& t) const;
        bool facetPush(const Fiber& f, Interval& i, const Triangle& t) const;
        bool edgePush(const Fiber& f, Interval& i, const Triangle& t) const;
        
        /// convert input radius r to cutter index
        unsigned int radius_to_index(double r) const;
        unsigned int height_to_index(double h) const;
        
        /// return true if radius=r belongs to cutter n
        bool validRadius(unsigned int n, double r) const;
        bool validHeight(unsigned int n, double h) const;

        double height(double r) const;
        double width(double h) const;
        
        /// return true if cl.cc is within the radial range of cutter n
        /// for cutter n the valid radial distance from cl is
        /// between radiusvec[n-1] and radiusvec[n]
        bool ccValidRadius(unsigned int n, CLPoint& cl) const;
        
        bool ccValidHeight(unsigned int n, CCPoint& cc, const Fiber& f) const;
        
        /// vector that holds the radiuses of the different cutters.
        /// cutter[0] is valid from r=0 to  r=radiusvec[0]
        /// cutter[1] is valid from r=radiusvec[0] to r=radiusvec[1]
        /// etc
        std::vector<double> radiusvec; // vector of radiuses
        /// vector for cutter heights
        /// cutter[0] is valid in h = [0, heihgtvec[0] ]
        /// cutter[1] is valid in h = [ heightvec[0], heihgtvec[1] ]
        std::vector<double> heightvec; // vector of heights
        /// vector of the axial offsets 
        std::vector<double> zoffset; // vector of z-offset values for the cutters
        /// vector of cutters in this CompositeCutter
        std::vector<MillingCutter*> cutter; // vector of pointers to cutters
};

/// a composite cutter for testing, consisting only of a cylindrical cutter
class CompCylCutter : public CompositeCutter {
    public:
        CompCylCutter() {}
        CompCylCutter(double diam, double length);
};

/// for testing, a single ballcutter
class CompBallCutter : public CompositeCutter {
    public:
        CompBallCutter() {}
        CompBallCutter(double diam, double length);
};

/// \brief CompositeCutter with a cylindrical/flat central part of diameter diam1
/// and a conical outer part sloping at angle, with a max diameter diam2
class CylConeCutter : public CompositeCutter {
    public:
        CylConeCutter() {}; // dummy, required(?) by python wrapper
        /// create cylconecutter
        CylConeCutter(double diam1, double diam2, double angle);
};

/// \brief CompositeCutter with a spherical central part of diameter diam1
/// and a conical outer part sloping at angle, with a max diameter diam2
/// the cone is positioned so that the tangent of the cone matches the tangent of the sphere
class BallConeCutter : public CompositeCutter {
    public:
        BallConeCutter() {}; // dummy, required(?) by python wrapper
        /// create ballconecutter
        BallConeCutter(double diam1, double diam2, double angle);
};

/// \brief CompositeCutter with a toroidal central part of diameter diam1 
/// and corner radius radius1
/// The outer part is conical sloping at angle, with a max diameter diam2
/// the cone is positioned so that the tangent of the cone matches the tangent of the torus
class BullConeCutter : public CompositeCutter {
    public:
        BullConeCutter() {}; // dummy, required(?) by python wrapper
        /// create bullconecutter
        BullConeCutter(double diam1, double radius1, double diam2, double angle);
};

/// \brief CompositeCutter with a conical central part with diam1/angle1 
/// and a conical outer part with diam2/angle2
/// we assume angle2 < angle1  and  diam2 > diam1.
class ConeConeCutter : public CompositeCutter {
    public:
        ConeConeCutter() {}; // dummy, required(?) by python wrapper
        /// create cone-cone cutter with lower cone (diam1,angle1) and upper cone (diam2,angle2)
        /// we assume angle2 < angle1  and  diam2 > diam1.
        ConeConeCutter(double diam1, double angle1, double diam2, double angle2);
};


} // end namespace
#endif
