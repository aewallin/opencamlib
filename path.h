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
#ifndef PATH_H
#define PATH_H

#include <boost/foreach.hpp>
#include <boost/python.hpp>
#include <list>
#include "point.h"
#include "line.h"
#include "arc.h"

///
/// \brief a path, simply a list of lines and arcs
///

///
/// longer documentation here.
///

enum SpanType{
	LineSpanType,
	ArcSpanType,
};

class Span{
public:
	virtual SpanType type()const = 0;

	virtual double length2d()const = 0;
	virtual Point getPoint(double fraction)const = 0; // 0.0 to 1.0
};

class LineSpan : public Span {
	public:
		LineSpan(const Line& l) : line(l){}
		Line line;

		// Span's virtual functions
		SpanType type()const{return LineSpanType;}
		double length2d()const{return line.length2d();}
		Point getPoint(double fraction)const{return line.getPoint(fraction);}
};

class ArcSpan : public Span {
	public:
		ArcSpan(const Arc& a) : arc(a){}
		Arc arc;

		// Span's virtual functions
		SpanType type()const{return ArcSpanType;}
		double length2d()const{return arc.length2d();}
		Point getPoint(double fraction)const{return arc.getPoint(fraction);}
};

class Path {
    public:
		Path();
        Path(const Path &p);
		~Path();

        boost::python::list getSpans();
		std::list<Span*> span_list;

		void append(const Line &l);
		void append(const Arc &a);
};

#endif
