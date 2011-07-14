//  stlreader.h
// 
//  Copyright 2010 Anders Wallin (anders.e.e.wallin "at" gmail.com)
//  
//  This file is part of OpenCAMlib.
//
//  OpenCAMlib is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  OpenCAMlib is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with OpenCAMlib.  If not, see <http://www.gnu.org/licenses/>.
//
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
