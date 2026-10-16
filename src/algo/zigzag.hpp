/*  $Id$
 * 
 *  Copyright (c) 2010 Anders Wallin (anders.e.e.wallin "at" gmail.com).
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

#ifndef ZIGZAG_H
#define ZIGZAG_H

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <cassert>

#include "bbox.hpp"
#include "point.hpp"

namespace ocl
{

/// zigzag 2D operation
class ZigZag {
    public:
        ZigZag() { }
        virtual ~ZigZag() {}
        /// step over distance 
        void setStepOver(double d) {
            stepOver = d;
        }
        /// set dir
        void setDirection(Point d) {
            dir = d;
        }
        void setOrigin(Point d) {
            origin = d;
        }
        
        /// run the algorithm
        void run() {
            // calculate a reasonable maximum/minimum step-over dist
            Point perp = dir.xyPerp();
            perp.xyNormalize();
            // std::cout << " minpt = " << bb.minpt << std::endl;
            // std::cout << " maxpt = " << bb.maxpt << std::endl;
            // std::cout << " perp = " << perp << std::endl;
            double max_d = (bb.maxpt - origin).dot( perp );
            double min_d = (bb.minpt - origin).dot( perp );
            if ( max_d < min_d ) {
                double tmp = max_d;
                max_d = min_d;
                min_d = tmp;
            }
            //int n = min_d / stepOver; // some safety margin here... (required?)
            // std::cout << " max_d= " << max_d << " min_d= "<< min_d << std::endl;
            
            std::vector<double> distances;
            for (double d = min_d ; d <= max_d ; d += stepOver ) {
                distances.push_back(d);
                out.push_back( origin + d*perp );
                assert( out.size() < 500 );
            }
            
        }
        
        /// add an input CLPoint to this Operation
        void addPoint(Point& p) {
            pocket.push_back(p);
            bb.addPoint(p);
        }
        std::string str() const {
            std::ostringstream o;
            o << "ZigZag: pocket.size()=" << pocket.size() << std::endl;
            return o.str();
        }
        auto getOutput() const { return out; }

    protected:
        /// the step over
        double stepOver;
        /// direction 
        Point dir;
        /// origin
        Point origin;
        /// pocket
        std::vector<Point> pocket;
        std::vector<Point> out;
        Bbox bb;
};

} // end namespace

#endif // end zigzag.h
