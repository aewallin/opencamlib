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

#include <boost/python.hpp>
#include <boost/python/docstring_options.hpp>

// #include "revision.h"



std::string ocl_docstring() {
    return "OpenCAMLib docstring";
}

namespace bp = boost::python;

void export_cutters();
void export_geometry();
// void export_cutsim(); // No python wrapping of cutsim, at least for now
void export_algo();

// this defines the python ocl module
BOOST_PYTHON_MODULE(ocl) {
    bp::docstring_options doc_options();
    // these functions set the docstring options
    
    //void disable_user_defined();
    void enable_user_defined();
    //void disable_signatures();
    void enable_signatures();
    //void disable_py_signatures();
    void enable_py_signatures();
    //void disable_cpp_signatures();
    void enable_cpp_signatures();
    //void disable_all();
    //void enable_all();
    
    bp::def("__doc__", ocl_docstring);

    export_geometry(); // see ocl_geometry.cpp

    export_cutters(); // see ocl_cutters.cpp

    //export_cutsim(); // see ocl_cutsim.cpp
    
    export_algo(); // see ocl_algo.cpp

}



