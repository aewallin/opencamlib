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

#ifndef VOLUME_H
#define VOLUME_H

#include <iostream>
#include <list>
#include <vector>

class Point;
class Triangle;
class MillingCutter;

/// base-class for defining volumes to build octrees
class OCTVolume {
    public:
        /// return true if Point p is inside volume
        virtual bool isInside(Point& p) const = 0;
};

/* required wrapper class for virtual functions in boost-python */
/// \brief a wrapper around OCTVolume required for boost-python
class OCTVolumeWrap : public OCTVolume, public bp::wrapper<OCTVolume>
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
        SphereOCTVolume();
        Point center;
        double radius;
        bool isInside(Point& p) const;
};

/// cube at center with side-length side
class CubeOCTVolume: public OCTVolume {
    public:
        CubeOCTVolume();
        Point center;
        double side;
        bool isInside(Point& p) const;
};

class CylinderOCTVolume: public OCTVolume {
    public:
        CylinderOCTVolume();
        Point p1;
        Point p2;
        double radius;
        bool isInside(Point& p) const;
};


#endif
