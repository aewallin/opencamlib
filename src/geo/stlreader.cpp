//  stlreader.cpp
// 
//  Written by Dan Heeks
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
#include <fstream>  // required by read_from_file()
#include <sstream>
#include <cstring>
#include <list>

#include "stlreader.hpp"
#include "stlsurf.hpp"

namespace ocl
{

    STLReader::STLReader(const std::wstring &filepath, STLSurf& surface) {
        read_from_file(filepath.c_str(), surface);
    }

    STLReader::~STLReader() {
        //delete tris;
    }

    using namespace std;

    static std::string str_for_Ttc;

    /// consider renaming?
    const char* Ttc(const wchar_t* str)
    {
        // convert a wchar_t* string into a char* string
        str_for_Ttc.clear();
        while (*str)
            str_for_Ttc.push_back((char) *str++);
        return str_for_Ttc.c_str();
    }

    void STLReader::read_from_file(const wchar_t* filepath, STLSurf& surface) {
        // read the stl file
        std::ifstream ifs(Ttc(filepath), ios::binary);
        if(!ifs)return;

        char solid_string[6] = "aaaaa";
        ifs.read(solid_string, 5);
        if(ifs.eof())return;
        if(strcmp(solid_string, "solid"))
        {
            // try binary file read

            // read the header
            char header[81];
            header[80] = 0;
            memcpy(header, solid_string, 5);
            ifs.read(&header[5], 75);

            unsigned int num_facets = 0;
            ifs.read((char*)(&num_facets), 4);

            for(unsigned int i = 0; i<num_facets; i++)
            {
                float n[3];
                float x[3][3];
                ifs.read((char*)(n), 12);
                ifs.read((char*)(x[0]), 36);
                short attr;
                ifs.read((char*)(&attr), 2);
                surface.addTriangle(Triangle(Point(x[0][0], x[0][1], x[0][2]), 
                    Point(x[1][0], x[1][1], x[1][2]), 
                    Point(x[2][0], x[2][1], x[2][2])));
            }
        }
        else
        {
            // "solid" already found
            char str[1024] = "solid";
            ifs.getline(&str[5], 1024);
            //char title[1024];
            //if(sscanf(str, "solid %s", title) == 1)
            //m_title.assign(Ctt(title));

            float n[3];
            float x[3][3];
            char five_chars[6] = "aaaaa";

            int vertex = 0;

            while(!ifs.eof())
            {
                ifs.getline(str, 1024);

                int i = 0, j = 0;
                for(; i<5; i++, j++)
                {
                    if(str[j] == 0)break;
                    while(str[j] == ' ' || str[j] == '\t')j++;
                    five_chars[i] = str[j];
                }
                if(i == 5)
                {
                    if(!strcmp(five_chars, "verte"))
                    {
#ifdef WIN32
                        sscanf(str, " vertex %f %f %f", &(x[vertex][0]), &(x[vertex][1]), &(x[vertex][2]));
#else
                        std::istringstream ss(str);
                        ss.imbue(std::locale("C"));
                        while(ss.peek() == ' ') ss.seekg(1, ios_base::cur);
                        ss.seekg(std::string("vertex").size(), ios_base::cur);
                        ss >> x[vertex][0] >> x[vertex][1] >> x[vertex][2];
#endif
                        vertex++;
                        if(vertex > 2)vertex = 2;
                    }
                    else if(!strcmp(five_chars, "facet"))
                    {
#ifdef WIN32
                        sscanf(str, " facet normal %f %f %f", &(n[0]), &(n[1]), &(n[2]));
#else
                        std::istringstream ss(str);
                        ss.imbue(std::locale("C"));
                        while(ss.peek() == ' ') ss.seekg(1, ios_base::cur);
                        ss.seekg(std::string("facet normal").size(), ios_base::cur);
                        ss >> n[0] >> n[1] >> n[2];
#endif
                        vertex = 0;
                    }
                    else if(!strcmp(five_chars, "endfa"))
                    {
                        if(vertex == 2)
                        {
                            surface.addTriangle(Triangle(Point(x[0][0], x[0][1], x[0][2]), 
                                Point(x[1][0], x[1][1], x[1][2]), 
                                Point(x[2][0], x[2][1], x[2][2])));
                        }
                    }
                }
            }
        }
    }

}
