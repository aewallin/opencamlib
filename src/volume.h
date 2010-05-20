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

#ifndef VOLUME_H
#define VOLUME_H

#include <iostream>
#include <list>
#include <vector>
#include "bbox.h"

namespace ocl
{
    
class Point;
class Triangle;
class MillingCutter;
class CylCutter;
class Ocode;

/// base-class for defining volumes to build octrees
class OCTVolume {
    public:
        /// default constructor
        OCTVolume(){};
        
        /// return true if Point p is inside volume
        virtual bool isInside(Point& p) const = 0;
        /// return true if we are in the bounding box
        bool isInsideBB(Point& p) const;
        /// return true if the Ocode o is inside the volume
        bool isInsideBBo(Ocode& o) const;

        /// bounding-box
        Bbox bb;
};

/* required wrapper class for virtual functions in boost-python */
/// \brief a wrapper around OCTVolume required for boost-python
class OCTVolumeWrap : public OCTVolume, public boost::python::wrapper<OCTVolume>
{
    public:
    bool isInside(Point &p) const
    {
        return this->get_override("isInside")(p);
    }

};



/// sphere centered at center
class SphereOCTVolume: public OCTVolume {
    public:
        /// default constructor
        SphereOCTVolume();
        /// center Point of sphere
        Point center;
        /// radius of sphere
        double radius;
        bool isInside(Point& p) const;
        //bool isInsideBB(Point& p) const;
        /// update the Bbox
        void calcBB();
};

/// cube at center with side-length side
class CubeOCTVolume: public OCTVolume {
    public:
        /// default constructor
        CubeOCTVolume();
        /// center point of cube
        Point center;
        /// side length of cube
        double side;
        bool isInside(Point& p) const;
        /// update bounding-box
        void calcBB();
};

/// cylinder volume
class CylinderOCTVolume: public OCTVolume {
    public:
        /// default constructor
        CylinderOCTVolume();
        /// startpoint of cylinder
        Point p1;
        /// endpoint of cylinder
        Point p2;
        /// cylinder radius
        double radius;
        bool isInside(Point& p) const;
        /// update the bounding box
        void calcBB();
};

/// box-volume
/// from corner, go out in the three directions v1,v2,v3
/// interior points = corner + a*v1 + b*v2 + c*v3  
/// where a, b, c are in [0,1]
class BoxOCTVolume: public OCTVolume {
    public:
        /// default constructor
        BoxOCTVolume();
        /// one corner of the box
        Point corner;
        /// first vector from corner, to span the box
        Point v1;
        /// second vector
        Point v2;
        /// third vector
        Point v3;
        bool isInside(Point& p) const;
};

/// elliptic tube volume
class EtubeOCTVolume: public OCTVolume {
    public:
        /// default constructor
        EtubeOCTVolume();
        /// construct Etube with given parameters
        EtubeOCTVolume(Point& p1in, Point& p2in, Point& ain, Point& bin);
        /// start of tune
        Point p1; // start of move
        /// end of tube
        Point p2; // end of move
        /// ellipse a-axis
        Point a; // a-axis of ellipse
        /// ellipse b-axis
        Point b; // b-axis of ellipse
        bool isInside(Point& p) const;
};


/// cutter-swept volume of a CylCutter
class CylMoveOCTVolume: public OCTVolume {
    public:
        /// default constructor
        CylMoveOCTVolume() {};
        /// create a CylMoveOCTVolume with the speicifed cutter and startpoint p1 and endpoint p2
        CylMoveOCTVolume(const CylCutter& c, const Point& p1, const Point& p2);
        /// start CL-Point for this move
        Point p1;
        /// end CL-point for this move
        Point p2;
        /// the CylCutter for this move
        CylCutter c;
        /// a cylindrical volume representing the cutter at p1
        CylinderOCTVolume c1;
        /// a cylindrical volume representing the cutter at p2
        CylinderOCTVolume c2;
        /// an elliptic tube of the swept-volube
        EtubeOCTVolume etube;
        /// the box-part of the swept-volyme
        BoxOCTVolume box;
        bool isInside(Point& p) const;
};

} // end namespace
#endif
// end file volume.h
