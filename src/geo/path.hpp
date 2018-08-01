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
#ifndef PATH_H
#define PATH_H

#include <list>

#include "point.hpp"
#include "line.hpp"
#include "arc.hpp"

namespace ocl
{


/// Span type
enum SpanType{
    LineSpanType,
    ArcSpanType
};

/// \brief A finite curve which returns Point objects along its length.
///
/// location along span is based on a parameter t for which 0 <= t <= 1.0
class Span{
    public:
        /// return type of span
        virtual SpanType type()const = 0;
        /// return the length of the span in the xy-plane
        virtual double length2d()const = 0;
        /// return a point at parameter value 0 <= t <= 1.0
        virtual Point getPoint(double t) const = 0; // 0.0 to 1.0
        /// avoid gcc 4.7.1 delete-non-virtual-dtor error
        virtual ~Span(){}
};

/// Line Span
class LineSpan : public Span {
    public:
        /// create a line span from Line l
        LineSpan(const Line& l) : line(l){}
        /// the line
        Line line;

        // Span's virtual functions
        /// return span type
        SpanType type()const{return LineSpanType;}
        /// return span length
        double length2d() const {
            return line.length2d();
        }
        /// return point on span
        Point getPoint(double t) const {
            return line.getPoint(t);
        }
};

/// circular Arc Span
class ArcSpan : public Span {
    public:
        /// create span
        ArcSpan(const Arc& a) : arc(a){}
        /// arc
        Arc arc;

        // Span's virtual functions
        /// return type
        SpanType type()const{return ArcSpanType;}
        /// return length in xy-plane
        double length2d()const{return arc.length2d();}
        /// return a point on the span
        Point getPoint(double t)const{return arc.getPoint(t);}
};

///
/// \brief A collection of Span objects
///
class Path {
    public:
        /// create empty path
        Path();
        /// copy constructor
        Path(const Path &p);
        /// destructor
        virtual ~Path();

        /// list of spans in this path
        std::list<Span*> span_list;

        // FIXME: this looks wrong
        // should be only one append() that takes a Span
        /// append a Line to this path
        void append(const Line &l);
        /// append an Arc to this path
        void append(const Arc &a);
};

} // end namespace
#endif
// end file path.h
