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

#ifndef STLREADER_H
#define STLREADER_H

namespace ocl
{
    
class STLSurf;


/// \brief STL file reader, reads an STL file and calls addTriangle on the STLSurf
///

class STLReader {
    public:
        STLReader(){};
        /// construct with file name and surface to fill
        STLReader(const std::wstring &filepath, STLSurf& surface);
        /// destructor
        virtual ~STLReader();

    private:
        /// read STL-surface from file
        void read_from_file(const wchar_t* filepath, STLSurf& surface);
};

}

#endif
