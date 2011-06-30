/*  $Id$
 * 
 *  Copyright 2010-2011 Anders Wallin (anders.e.e.wallin "at" gmail.com)
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

#ifndef GL_DATA_H
#define GL_DATA_H

#include <QObject>
#include <QGLBuffer>
#include <QVarLengthArray>

#include <iostream>
#include <set>
#include <cmath>

#include <boost/foreach.hpp>
#include <boost/function.hpp>

#include "octnode.h"

namespace ocl
{

/// a vertex/point in 3D, with (x,y,z) coordinates of type GLfloat
/// normal is (nx,ny,nz)
/// color is (r,g,b)
struct GLVertex {
    GLVertex() : x(0), y(0), z(0), r(0), g(0), b(0) {}
    GLVertex(GLfloat x, GLfloat y, GLfloat z) 
         : x(x), y(y), z(z), r(0), g(0), b(0) {}
    GLVertex(GLfloat x, GLfloat y, GLfloat z, GLfloat r, GLfloat g, GLfloat b) 
         : x(x), y(y), z(z), r(r), g(g), b(b) {}
    void setNormal(GLfloat x, GLfloat y, GLfloat z) {
        nx=x;
        ny=y;
        nz=z;
        // normalize:
        GLfloat norm = sqrt( x*x+y*y+z*z );
        nx /= norm;
        ny /= norm;
        nz /= norm;
    }
    void str() {
        std::cout << "(" << x << ", " << y << ", " << z << ")"; 
    }
// DATA
    GLfloat x,y,z; // position
    GLfloat r,g,b; // color, 12-bytes offset from position data.
    GLfloat nx,ny,nz; // normal, 24-bytes offset
};

/// additional vertex data not needed for OpenGL rendering
/// but required for the isosurface or cutting-simulation algorithm.
struct VertexData {
    void str() {
        BOOST_FOREACH( GLuint pIdx, polygons ) {
            std::cout << pIdx << " ";
        }
    }

    inline void addPolygon( unsigned int idx ) { polygons.insert( idx ); }
    inline void removePolygon(unsigned int idx ) { polygons.erase( idx ); }
    inline bool empty() { return polygons.empty(); }

// DATA
    /// The set of polygons. Each polygon has an uint index which is stored here.
    /// Note: we want to access polygons from highest index to lowest, thus compare with "greater"
    typedef std::set< unsigned int, std::greater<unsigned int> > PolygonSet;
    /// the polygons to which this vertex belongs. i.e. for each vertex we store in this set all the polygons to which it belongs.
    PolygonSet polygons;
    
    /// the Octnode that created this vertex. 
    /// This allows the Octnode to delete the vertex if required (e.g. the Octnode is cut)
    Octnode* node;
    // (an alternative callback-mechanism would be to store a function-pointer or similar)
};




// the "secret sauce" paper suggests the following primitives
//   http://www.cs.berkeley.edu/~jrs/meshpapers/SchaeferWarren2.pdf
//   or
//   http://citeseerx.ist.psu.edu/viewdoc/summary?doi=10.1.1.13.2631
//
// - add vertex  
//   add vertex with empty polygon list and pointer to octree-node
//
// - remove vertex (also removes associated polygons)
//   process list of polygons, from highest to lowest. call remove_polygon on each poly.
//   overwrite with last vertex. shorten list. request each poly to re-number.
// 
// - add polygon DONE
//   append new polygon to end of list, request each vertex to add new polygon to list.
//
// - remove_polygon( polygonIndex ) 
//   i) for each vertex: request remove this polygons index from list
//   ii) then remove polygon from polygon-list: overwrite with last polygon, then shorten list.
//   iii) process each vertex in the moved polygon, request renumber on each vert for this poly
//
// data structure:
//  vertex-table: index, pos(x,y,z)  , polygons(id1,id2,...), Node-pointer to octree 
// polygon-table: index, vertex-list
//

/// a GLData object holds data which is drawn by OpenGL using VBOs
class GLData {
public:
    GLData() {
        // some reasonable defaults...
        type = GL_TRIANGLES;
        polyVerts = 3;
        polygonMode_face = GL_FRONT_AND_BACK;
        polygonMode_mode = GL_LINE;
        usagePattern = QGLBuffer::StaticDraw;
    }
    /// add a vertex with given position and color, return its index
    unsigned int addVertex(float x, float y, float z, float r, float g, float b);
    /// add vertex
    unsigned int addVertex(GLVertex v);
    /// add vertex, give position, color, Octnode*
    unsigned int addVertex(float x, float y, float z, float r, float g, float b, Octnode* n);
    
    /// for a given vertex, set the normal
    void setNormal(unsigned int vertexIdx, float nx, float ny, float nz) {
        vertexArray[vertexIdx].setNormal(nx,ny,nz);
    }
    
    /// remove vertex with given index
    void removeVertex( unsigned int vertexIdx );
    /// add a polygon, return its index
    int addPolygon( std::vector<GLuint>& verts);
    /// remove polygon at given index
    void removePolygon( unsigned int polygonIdx);
    /// return the number of polygons
    int polygonCount() const { return indexArray.size(); }
    
    /// generate the VBOs
    void genVBO();
    /// update VBO
    void updateVBO();
    
    /// set polygon type to Triangles
    void setTriangles() {setType(GL_TRIANGLES); polyVerts=3;}
    /// set polygon type to Quads
    void setQuads() {setType(GL_QUADS); polyVerts=4;}
    /// set type to Points
    void setPoints() {setType(GL_POINTS); polyVerts=1;}
    
    void setUsageStaticDraw() {usagePattern = QGLBuffer::StaticDraw;}
    void setUsageDynamicDraw() {usagePattern = QGLBuffer::DynamicDraw;}
    /// bind the vertex and index buffers
    bool bind();
    /// release the vertex and index buffers
    void release();
    void setPosition(float x, float y, float z);
    void print() ;
//DATA
    // the type of this GLData, one of:
    //                GL_POINTS,
    //                GL_LINE_STRIP,
    //                GL_LINE_LOOP,
    //                GL_LINES,
    //                GL_TRIANGLE_STRIP,
    //                GL_TRIANGLE_FAN,
    //                GL_TRIANGLES,
    //                GL_QUAD_STRIP,
    //                GL_QUADS,
    //                GL_POLYGON 
    GLenum type;
    
    // usagePattern is set to one of:
    //    QGLBuffer::StreamDraw         The data will be set once and used a few times for drawing operations. Under OpenGL/ES 1.1 this is identical to StaticDraw.
    //    QGLBuffer::StreamRead         The data will be set once and used a few times for reading data back from the GL server. Not supported under OpenGL/ES.
    //    QGLBuffer::StreamCopy         The data will be set once and used a few times for reading data back from the GL server for use in further drawing operations. Not supported under OpenGL/ES.
    //    QGLBuffer::StaticDraw         The data will be set once and used many times for drawing operations.
    //    QGLBuffer::StaticRead         The data will be set once and used many times for reading data back from the GL server. Not supported under OpenGL/ES.
    //    QGLBuffer::StaticCopy         The data will be set once and used many times for reading data back from the GL server for use in further drawing operations. Not supported under OpenGL/ES.
    //    QGLBuffer::DynamicDraw        The data will be modified repeatedly and used many times for drawing operations.
    //    QGLBuffer::DynamicRead        The data will be modified repeatedly and used many times for reading data back from the GL server. Not supported under OpenGL/ES.
    //    QGLBuffer::DynamicCopy        The data will be modified repeatedly and used many times for reading data back from the GL server for use in further drawing operations. Not supported under OpenGL/ES
    QGLBuffer::UsagePattern usagePattern;
    
    GLenum polygonMode_face; // face = GL_FRONT | GL_BACK  | GL_FRONT_AND_BACK
    GLenum polygonMode_mode; // mode = GL_POINT, GL_LINE, GL_FILL
    
    
        
    
    typedef GLVertex vertex_type;
    static const GLenum index_type = GL_UNSIGNED_INT;
    static const GLenum coordinate_type = GL_FLOAT;
    static const unsigned int vertex_offset = 0;
    static const unsigned int color_offset = 12;
    static const unsigned int normal_offset = 24;
    /// translation to be applied before drawing
    // fixme: use translation matrix instead.
    GLVertex pos;
    
protected:
    
    template <class Data>
    void updateBuffer(  QGLBuffer* buffer, Data& d) {
        if (!buffer->bind())
            assert(0);
        buffer->allocate( d.data(), sizeof(typename Data::value_type)*d.size() );
        buffer->release();
    }
    
    template <class Data>
    QGLBuffer* makeBuffer(  QGLBuffer::Type t, Data& d) {
        QGLBuffer* buffer = new QGLBuffer(t);
        buffer->create();
        if (!buffer->bind())
            assert(0);
        buffer->setUsagePattern( usagePattern );
        //std::cout << " allocating " << sizeof(typename Data::value_type)*d.size() << " bytes.\n";
        buffer->allocate( d.data(), sizeof(typename Data::value_type)*d.size() );
        //std::cout << " buffer size = " << buffer->size() << "\n";
        buffer->release();
        return buffer;
    }
    
    /// set type of drawing, e.g. GL_TRIANGLES, GL_QUADS
    void setType(GLenum t) {
        type = t;
    }
    /// set the OpenGL usage pattern
    void setUsage(QGLBuffer::UsagePattern p ) {
        usagePattern = p;
    }
    
// DATA
    /// vertex data buffer
    QGLBuffer* vertexBuffer;
    /// index data buffer
    QGLBuffer* indexBuffer;
    /// number of vertices per polygon. 1 for GL_POINTS, 3 for GL_TRIANGLES, 4 for GL_QUADS
    int polyVerts; 
    
    /// vertices stored in this array. this array is bound to the OpenGL buffer
    /// and used directly for drawing as the OpenGL vertex position, color, and normal array.
    QVarLengthArray<GLVertex> vertexArray;
    /// extra vertex data is stored here. this data is not needed for OpenGL drawing.
    /// but it is required for the isosurface-algorithms (marching-cubes / dual contouring)
    QVarLengthArray<VertexData> vertexDataArray;
    /// this is the index array for drawing polygons. used by OpenGL glDrawElements
    QVarLengthArray<GLuint> indexArray;
};

} // end namespace

#endif
