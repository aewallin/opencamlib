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
#include <array>
#include <codecvt>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <locale>
#include <stdexcept>
#include <string>

#include "stlreader.hpp"
#include "stlsurf.hpp"

namespace ocl {
namespace {
std::uint32_t little_u32(const unsigned char* bytes) {
    return std::uint32_t(bytes[0]) | (std::uint32_t(bytes[1]) << 8) |
           (std::uint32_t(bytes[2]) << 16) | (std::uint32_t(bytes[3]) << 24);
}
float little_float(const unsigned char* bytes) {
    static_assert(sizeof(float) == 4, "STL requires 32-bit floats");
    const auto bits = little_u32(bytes);
    float value;
    std::memcpy(&value, &bits, sizeof(value));
    return value;
}
void expect(std::istream& stream, const char* expected) {
    std::string token;
    if (!(stream >> token) || token != expected)
        throw std::runtime_error(std::string("Invalid ASCII STL: expected ") + expected);
}
Point read_point(std::istream& stream) {
    double x, y, z;
    if (!(stream >> x >> y >> z) || !std::isfinite(x) || !std::isfinite(y) || !std::isfinite(z))
        throw std::runtime_error("Invalid ASCII STL coordinates");
    return Point(x, y, z);
}
void read_ascii(std::istream& stream, STLSurf& result) {
    // Solid names are line-based; facet fields may be separated by any whitespace.
    expect(stream, "solid");
    for (;;) {
        std::string line, token;
        std::getline(stream, line);
        bool ended = false;
        while (stream >> token) {
            if (token == "endsolid") {
                std::getline(stream, line);
                ended = true;
                break;
            }
            if (token != "facet") throw std::runtime_error("Invalid ASCII STL: expected facet");
            expect(stream, "normal");
            read_point(stream); // Recompute normals from vertices.
            expect(stream, "outer"); expect(stream, "loop");
            Point vertices[3];
            for (auto& vertex : vertices) {
                expect(stream, "vertex"); vertex = read_point(stream);
            }
            expect(stream, "endloop"); expect(stream, "endfacet");
            result.addTriangle(Triangle(vertices[0], vertices[1], vertices[2]));
        }
        if (!ended) throw std::runtime_error("Truncated ASCII STL: missing endsolid");
        stream >> std::ws;
        if (stream.eof()) return;
        expect(stream, "solid"); // Multiple solids are permitted.
    }
}
} // namespace

STLReader::STLReader(const std::wstring& filepath, STLSurf& surface) {
    read_from_file(filepath.c_str(), surface);
}
STLReader::~STLReader() = default;

void STLReader::read_from_file(const wchar_t* filepath, STLSurf& surface) {
#ifdef _WIN32
    std::ifstream stream(filepath, std::ios::binary);
#else
    const auto filename = std::wstring_convert<std::codecvt_utf8<wchar_t>>().to_bytes(filepath);
    std::ifstream stream(filename, std::ios::binary);
#endif
    if (!stream) throw std::runtime_error("Cannot open STL file");
    stream.imbue(std::locale::classic());
    stream.seekg(0, std::ios::end);
    const auto length = stream.tellg();
    if (length < 0) throw std::runtime_error("Cannot determine STL file size");
    stream.seekg(0);
    std::array<unsigned char, 84> header{};
    stream.read(reinterpret_cast<char*>(header.data()), header.size());
    const auto count = little_u32(header.data() + 80);
    const bool binary = length >= 84 &&
        static_cast<std::uint64_t>(length) == 84ULL + 50ULL * count;
    stream.clear();
    stream.seekg(binary ? 84 : 0);
    STLSurf parsed;
    try {
        if (binary) {
            for (std::uint32_t i = 0; i < count; ++i) {
                std::array<unsigned char, 50> facet{};
                if (!stream.read(reinterpret_cast<char*>(facet.data()), facet.size()))
                    throw std::runtime_error("Truncated binary STL facet");
                Point vertices[3];
                for (int j = 0; j < 3; ++j) {
                    const auto* vertex = facet.data() + 12 + j * 12;
                    vertices[j] = Point(little_float(vertex), little_float(vertex + 4), little_float(vertex + 8));
                }
                parsed.addTriangle(Triangle(vertices[0], vertices[1], vertices[2]));
            }
        } else {
            read_ascii(stream, parsed);
        }
    } catch (const std::invalid_argument& error) {
        throw std::runtime_error(std::string("Invalid STL geometry: ") + error.what());
    }
    // Commit only after the entire file has been validated.
    for (const auto& triangle : parsed.tris) surface.bb.addTriangle(triangle);
    surface.tris.splice(surface.tris.end(), parsed.tris);
}
} // namespace ocl
