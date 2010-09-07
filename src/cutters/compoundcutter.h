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

#ifndef COMPOUND_CUTTER_H
#define COMPOUND_CUTTER_H

#include <iostream>
#include <string>
#include <vector>

#include "millingcutter.h"

namespace ocl
{

/* ********************************************************************
 *  CompoundCutter base class
 * ********************************************************************/
/// \brief a CompoundCutter is composed one or more MillingCutters
/// the cutters are stored in a vector *cutter* and their axial offsets
/// from eachother in *zoffset*. The different cutters apply in different
/// radial regions. cutter[0] from r=0 to r=radius[0] after that 
/// cutter[1] from r=radius[0] to r=radius[1] and so on. 
class CompoundCutter : public MillingCutter {
    public:
        /// create an empty CompoundCutter
        CompoundCutter();
        /// add a MillingCutter to this CompoundCutter
        /// the cutter is valid from the previous radius out to radius
        /// and its axial offset is given by zoffset
        void addCutter(MillingCutter& c, double radius, double zoff);
        /// return true if cc_tmp is in the valid region of cutter n
        bool ccValid(int n, CLPoint& cl) const;
        
        // offsetCutter
        MillingCutter* offsetCutter(const double d) const;
        
        // dropCutter methods
        bool vertexDrop(CLPoint &cl, const Triangle &t) const;
        bool facetDrop(CLPoint &cl, const Triangle &t) const;
        bool edgeDrop(CLPoint &cl, const Triangle &t) const;
        
        // string output
        std::string str() const;
        
    protected:        
        /// vector that holds the radiuses of the different cutters
        std::vector<double> radiusvec; // vector of radiuses
        /// vector of the axial offsets 
        std::vector<double> zoffset; // vector of z-offset values for the cutters
        /// vector of cutters in this CompoundCutter
        std::vector<MillingCutter*> cutter; // vector of pointers to cutters
};




/* ********************************************************************
 *  different cutter shapes
 * ********************************************************************/

/// \brief a MillingCutter::CompoundCutter with a cylindrical/flat central part of diameter diam1
/// and a conical outer part sloping at angle, with a max diameter diam2
class CylConeCutter : public CompoundCutter {
    public:
        CylConeCutter() {}; // dummy, required(?) by python wrapper
        CylConeCutter(double diam1, double diam2, double angle);
};

/// \brief a MillingCutter::CompoundCutter with a spherical central part of diameter diam1
/// and a conical outer part sloping at angle, with a max diameter diam2
/// the cone is positioned so that the tangent of the cone matches the tangent of the sphere
class BallConeCutter : public CompoundCutter {
    public:
        BallConeCutter() {}; // dummy, required(?) by python wrapper
        BallConeCutter(double diam1, double diam2, double angle);
};

/// \brief a MillingCutter::CompoundCutter with a toroidal central part of diameter diam1 
/// and corner radius radius1
/// The outer part is conical sloping at angle, with a max diameter diam2
/// the cone is positioned so that the tangent of the cone matches the tangent of the torus
class BullConeCutter : public CompoundCutter {
    public:
        BullConeCutter() {}; // dummy, required(?) by python wrapper
        BullConeCutter(double diam1, double radius1, double diam2, double angle);
};

/// \brief a MillingCutter::CompoundCutter with a conical central part with diam1/angle1 
/// and a conical outer part with diam2/angle2
class ConeConeCutter : public CompoundCutter {
    public:
        ConeConeCutter() {}; // dummy, required(?) by python wrapper
        ConeConeCutter(double diam1, double angle1, double diam2, double angle2);
};


} // end namespace
#endif
