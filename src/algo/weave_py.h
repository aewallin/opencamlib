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
#ifndef WEAVE_PY_H
#define WEAVE_PY_H

#include <boost/python.hpp> // for py
#include <boost/foreach.hpp> // for py

#include "weave.h"

namespace ocl
{

/// Python wrapper for Weave
class Weave_py : public Weave {
    public:
        Weave_py() : Weave() {};
    // PYTHON
        /// return graph components to python
        boost::python::list get_components() {
            boost::python::list wlist;
            std::vector<Weave> weaves = split_components();
            BOOST_FOREACH( Weave w, weaves ) {
                wlist.append( w );
            }
            return wlist;
        };
        /// return CL-points to python
        boost::python::list getCLPoints() const {
            boost::python::list plist;
            VertexIterator it_begin, it_end, itr;
            boost::tie( it_begin, it_end ) = boost::vertices( g );
            for ( itr=it_begin ; itr != it_end ; ++itr ) {
                if ( g[*itr].type == CL )
                    plist.append( g[*itr].position );
                    
                //if ( boost::get( boost::vertex_type, g, *itr ) == CL ) // a CL-point
                //    plist.append( boost::get( boost::vertex_position, g, *itr ) );
            }
            return plist;
        };
        /// return internal points to python
        boost::python::list getIPoints() const {
            boost::python::list plist;
            VertexIterator it_begin, it_end, itr;
            boost::tie( it_begin, it_end ) = boost::vertices( g );
            for ( itr=it_begin ; itr != it_end ; ++itr ) {
                if ( g[*itr].type == INT )
                    plist.append( g[*itr].position );
                    
                //if ( boost::get( boost::vertex_type, g, *itr ) == INT ) 
                //    plist.append( boost::get( boost::vertex_position, g, *itr ) );
            }
            return plist;
        };
        /// return edges to python
        /// format is [ [p1,p2] , [p3,p4] , ... ]
        boost::python::list getEdges() const {
            boost::python::list edge_list;
            EdgeIterator it_begin, it_end, itr;
            boost::tie( it_begin, it_end ) = boost::edges( g );
            for ( itr=it_begin ; itr != it_end ; ++itr ) { // loop through each edge
                if ( ! boost::get( boost::edge_color, g, *itr ) ) {
                    boost::python::list point_list; // the endpoints of each edge
                    WeaveVertex v1 = boost::source( *itr, g  );
                    WeaveVertex v2 = boost::target( *itr, g  );
                    Point p1 = g[v1].position; // boost::get( boost::vertex_position, g, v1 );
                    Point p2 = g[v2].position; // boost::get( boost::vertex_position, g, v2 );
                    point_list.append(p1);
                    point_list.append(p2);
                    edge_list.append(point_list);
                }
            }
            return edge_list;
        };
        /// return loops to pyton
        boost::python::list py_getLoops() const {
            boost::python::list loop_list;
            BOOST_FOREACH( std::vector<WeaveVertex> loop, loops ) {
                boost::python::list point_list;
                BOOST_FOREACH( WeaveVertex v, loop ) {
                    //Point p = boost::get( boost::vertex_position, g, v);
                    point_list.append( g[v].position );
                }
                loop_list.append(point_list);
            }
            return loop_list;
        };
};

} // end namespace
#endif
// end file weave_py.h
