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
#ifndef PATH_PY_H
#define PATH_PY_H

#include <list>

#include <boost/python.hpp>
#include <boost/foreach.hpp>

#include "path.hpp"



namespace ocl
{

/// Python wrapper for Path
class Path_py : public Path {
    public:
        Path_py() : Path () {};
        /// copy constructor
        Path_py(const Path &p) : Path(p) {};
        
        /// return the span-list to python
        boost::python::list getSpans() {
            boost::python::list slist;
            BOOST_FOREACH(Span* span, span_list) {
                        if(span->type() == LineSpanType)slist.append(((LineSpan*)span)->line);
                        else if(span->type() == ArcSpanType)slist.append(((ArcSpan*)span)->arc);
            }
            return slist;
        };

        /// return a list of type/span pairs
        boost::python::list getTypeSpanPairs() {
            boost::python::list slist;
            BOOST_FOREACH(Span* span, span_list) {
                        if(span->type() == LineSpanType)
                        {
                            boost::python::list tuple;
                            tuple.append(span->type());
                            tuple.append(((LineSpan*)span)->line);
                            slist.append(tuple);
                        }
                        else if(span->type() == ArcSpanType)
                        {
                            boost::python::list tuple;
                            tuple.append(span->type());
                            tuple.append(((ArcSpan*)span)->arc);
                            slist.append(tuple);
                        }
            }
            return slist;
        };
    
    
};

} // end namespace
#endif
// end file path.h
