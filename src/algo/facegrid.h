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

#ifndef FACEGRID_H
#define FACEGRID_H

#include <boost/multi_array.hpp> // for bucketing in FaceGrid

#include "halfedgediagram.h"


namespace ocl
{

typedef std::vector<FaceProps> FacePropVector;
typedef boost::multi_array< FacePropVector* , 2> Grid;
typedef Grid::index GridIndex;


/// a VoronoiFace list which is updated when we build the voronoi diagram
class FaceGrid {
    public:
        FaceGrid();
        FaceGrid(double far, unsigned int n_bins);
        void add_face(FaceProps props);
        HEFace find_closest_face(const Point& p);
        HEFace grid_find_closest_face(const Point& p);

    private:     
        GridIndex get_grid_index( double x );
        HEFace find_closest_in_set( std::set<FaceProps>& set, const Point&p );
        void insert_faces_from_neighbors( std::set<FaceProps>& set, GridIndex row, GridIndex col , GridIndex dist );
        void insert_faces_from_bucket( std::set<FaceProps>& set, GridIndex row, GridIndex col );
    // DATA
        double far_radius;
        double binwidth;
        GridIndex nbins;
        Grid* grid;
};


}
#endif
// end facegrid.h
