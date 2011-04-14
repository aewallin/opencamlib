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

// additional data not needed for OpenGL rendering
// but required for the isosurface or cutting-simulation algorithm.
//typedef boost::function2< void, unsigned int, unsigned int> VoidIntIntCallBack;
struct VertexData {
    void str() {
        BOOST_FOREACH( GLuint pIdx, polygons ) {
            std::cout << pIdx << " ";
        }
    }
    // Note: we want to access polygons from highest index to lowest, thus compare with "greater"
    typedef std::set< unsigned int, std::greater<unsigned int> > PolygonSet;
    
    void addPolygon( unsigned int idx ) {
        polygons.insert( idx );
    }
    void removePolygon(unsigned int idx ) {
        polygons.erase( idx );
    }
    bool empty() {
        return polygons.empty();
    }
    // TODO: octree-node pointer.
    
// DATA
    PolygonSet polygons;
    
    // this function is called if the vertex indices change.
    // intended use: each octree-node stores the indexes of the vertices it has produced
    // to keep this data valid, each node is notified whenever GLData reorders vertices.
    // the call is: void callBack( oldIndex, newIndex )
    //VoidIntIntCallBack indexSwapCallBack;
    Octnode* node;
    
    //OctreeNode* node; // pointer to the octree-node that generated this vertex
};




// the "secret sauce" paper suggests the following primitives
//
// - add vertex  DONE
//   add vertex with empty polygon list and pointer to octree-node
//
// - remove vertex (also removes associated polygons)
//   process list of polygons, from highest to lowest. call remove_polygon on each poly.
//   overwrite with last vertex. shorten list. request each poly to re-number
// 
// - add polygon DONE
//   append new polygon to end of list, request each vertex to add new polygon to list.
//
// - remove_polygon( polygonIndex ) DONE
//   i) for each vertex: request remove this polygons index from list
//   ii) then remove polygon from polygon-list: overwrite with last polygon, then shorten list.
//   iii) process each vertex in the moved polygon, request renumber on each vert for this poly
//
// data structure:
// vertex: index, pos(x,y,z), polygons(id1,id2,...), Node-pointer(to octree)
// polygon: index, vertex-list
//

/// a GLData object holds data which is drawn by OpenGL using VBOs
class GLData {
public:
    /// add a vertex, return its index
    int addVertex(float x, float y, float z, float r, float g, float b);
    int addVertex(GLVertex v);

    //int addVertex(float x, float y, float z, float r, float g, float b, VoidIntIntCallBack c);
    void setNormal(unsigned int vertexIdx, float x, float y, float z) {
        vertexArray[vertexIdx].setNormal(x,y,z);
    }
    int addVertex(float x, float y, float z, float r, float g, float b, Octnode* n);
    
    /// remove vertex at given index
    void removeVertex( unsigned int vertexIdx );
    /// add a polygon, return its index
    int addPolygon( std::vector<GLuint>& verts);
    /// remove polygon at given index
    void removePolygon( unsigned int polygonIdx);
    /// return the number of polygons
    int indexCount() const { return indexArray.size(); }
    
    

    template <class Data>
    void updateBuffer(  QGLBuffer* buffer, Data& d) {
        if (!buffer->bind())
            assert(0);
        buffer->allocate( d.data(), sizeof(typename Data::value_type)*d.size() );
        buffer->release();
    }
    
    /// generate the VBOs
    void genVBO();
    void updateVBO();
    /// set polygon type to triangles
    void setTriangles() {setType(GL_TRIANGLES); polyVerts=3;}
    /// set polygon type to quads
    void setQuads() {setType(GL_QUADS); polyVerts=4;}
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
    
    typedef GLVertex vertex_type;
    static const GLenum index_type = GL_UNSIGNED_INT;
    static const GLenum coordinate_type = GL_FLOAT;
    static const unsigned int vertex_offset = 0;
    static const unsigned int color_offset = 12;
    static const unsigned int normal_offset = 24;
    /// translation to be applied before drawing
    GLVertex pos;
    
protected:
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
    QGLBuffer* vertexBuffer;
    QGLBuffer* indexBuffer;
    /// number of vertices per polygon. 3 for GL_TRIANGLES, 4 for GL_QUADS
    int polyVerts; 
    
    // vertices stored in this array. this array is bound to the OpenGL buffer
    // and used directly for drawing as the OpenGL vertex, color, and normal array.
    QVarLengthArray<GLVertex> vertexArray;
    // extran vertex data is stored here. this data is not needed for OpenGL drawing.
    QVarLengthArray<VertexData> vertexDataArray;
    // this is the index array for drawing polygons. used by OpenGL glDrawElements
    QVarLengthArray<GLuint> indexArray;
};

} // end namespace

#endif
