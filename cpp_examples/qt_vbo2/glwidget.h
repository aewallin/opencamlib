#ifndef GLWIDGET
#define GLWIDGET

#include <QObject>
#include <QGLBuffer>
#include <QVarLengthArray>
#include <QtGui>

#include <boost/foreach.hpp> 

#include <iostream>
#include <cassert>
#include <set>
#include <vector>

#define BUFFER_OFFSET(i) ((GLbyte *)NULL + (i))

struct GLVertex {
    GLVertex() : x(0), y(0), z(0), r(0), g(0), b(0) {}
    GLVertex(GLfloat x, GLfloat y, GLfloat z, GLfloat r, GLfloat g, GLfloat b) 
         : x(x), y(y), z(z), r(r), g(g), b(b) {}
    //friend std::ostream& operator<<(std::ostream &stream, const GLVertex &p);
    void str() {
        std::cout << "(" << x << ", " << y << ", " << z << ")"; 
    }
// DATA
    GLfloat x,y,z; // position
    GLfloat r,g,b; // color, 12-bytes offset from position data.
};

/*
std::ostream& operator<<(std::ostream &stream, const GLVertex& p) {
    stream << "(" << p.x << ", " << p.y << ", " << p.z << ")"; 
    return stream;
}*/

// additional data not needed for OpenGL rendering
// but required for the algorithm.
struct VertexData {
    //friend std::ostream& operator<<(std::ostream &stream, const VertexData &d);
    void str() {
        BOOST_FOREACH( GLuint pIdx, polygons ) {
            std::cout << pIdx << " ";
        }
    }
    // Note: we want to access polygons from highest index to lowest, thus compare with "greater"
    typedef std::set< unsigned int, std::greater<unsigned int> > PolygonSet;
    PolygonSet polygons;
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
};

/*
std::ostream& operator<<(std::ostream &stream, const VertexData& vdata) {

    //stream << "(" << p.x << ", " << p.y << ", " << p.z << ")"; 
    return stream;
}*/


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
struct GLData {
    int addVertex(GLVertex v) {
        // add vertex with empty polygon-list.
        int idx = vertexArray.size();
        vertexArray.append(v);
        vertexDataArray.append( VertexData() );
        assert( vertexArray.size() == vertexDataArray.size() );
        return idx; // return index of newly appended vertex
    }
    void removeVertex( unsigned int vertexIdx ) {
        // i) for each polygon of this vertex, call remove_polygon:
        BOOST_FOREACH( unsigned int polygonIdx, vertexDataArray[vertexIdx].polygons ) {
            removePolygon( polygonIdx );
        }
        // ii) overwrite with last vertex:
        unsigned int lastIdx = vertexArray.size()-1;
        if (vertexIdx != lastIdx) {
            vertexArray[vertexIdx] = vertexArray[lastIdx];
            vertexDataArray[vertexIdx] = vertexDataArray[lastIdx];
            // update octree-node with new index here!
            
            // request each polygon to re-number this vertex.
            BOOST_FOREACH( unsigned int polygonIdx, vertexDataArray[vertexIdx].polygons ) {
                unsigned int idx = polygonIdx*3;
                if ( indexArray[ idx ] == lastIdx )
                    indexArray[ idx ] = vertexIdx;
                if ( indexArray[ idx+1 ] == lastIdx )
                    indexArray[ idx+1 ] = vertexIdx;
                if ( indexArray[ idx+2 ] == lastIdx )
                    indexArray[ idx+2 ] = vertexIdx;
            }
        }
        // shorten array
        vertexArray.resize( vertexArray.size()-1 );
        vertexDataArray.resize( vertexDataArray.size()-1 );
        assert( vertexArray.size() == vertexDataArray.size() );
    }
    int addPolygon( std::vector<GLuint>& verts) {
        // append to indexArray, request each vertex to update
        unsigned int polygonIdx = indexArray.size()/3;
        BOOST_FOREACH( GLuint vertex, verts ) {
            indexArray.append(vertex);
            vertexDataArray[vertex].addPolygon(polygonIdx); // add index to vertex i1
        }
        return polygonIdx;
    }
    //void removePolygonFromVertex( unsigned int vertexIdx, unsigned int polygonIdx) {
    //    if ( vertexDataArray[ indexArray[vertexIdx]   ].removePolygon(polygonIdx) ) {
    //        removeVertex( vertexIdx );
    //    }
    //}
    void removePolygon( unsigned int polygonIdx) {
        std::cout << " removePolygon( " << polygonIdx << " )\n";
        unsigned int idx = 3*polygonIdx; // start-index for polygon
        // i) request remove for each vertex in polygon:
        vertexDataArray[ indexArray[idx]   ].removePolygon(polygonIdx);
        vertexDataArray[ indexArray[idx+1] ].removePolygon(polygonIdx);
        vertexDataArray[ indexArray[idx+2] ].removePolygon(polygonIdx);
        // check for orphan vertices (?)
        unsigned int last_index = (indexArray.size()-3);
        if (idx!=last_index) { // if deleted polygon is last on the list, do nothing??
        
            // ii) remove from polygon-list by overwriting with last element
            indexArray[idx  ] = indexArray[ last_index   ];
            indexArray[idx+1] = indexArray[ last_index+1 ];
            indexArray[idx+2] = indexArray[ last_index+2 ];
            
            // iii) for the moved polygon, request that each vertex update the polygon number
            vertexDataArray[ indexArray[idx   ] ].addPolygon( idx/3 ); // this is the new polygon index
            vertexDataArray[ indexArray[idx+1 ] ].addPolygon( idx/3 );
            vertexDataArray[ indexArray[idx+2 ] ].addPolygon( idx/3 );
            
            vertexDataArray[ indexArray[idx   ] ].removePolygon( last_index/3 ); // this polygon is no longer there!
            vertexDataArray[ indexArray[idx+1 ] ].removePolygon( last_index/3 );
            vertexDataArray[ indexArray[idx+2 ] ].removePolygon( last_index/3 );
        }
        
        // shorten array
        indexArray.resize( indexArray.size()-3 );
        

    } 
    int indexCount() const {
        return indexArray.size();
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
    
    void genVBO() {
        vertexBuffer = makeBuffer(  QGLBuffer::VertexBuffer, vertexArray );
        indexBuffer = makeBuffer( QGLBuffer::IndexBuffer, indexArray );
    }
    
    void setType(GLenum t) {
        type = t;
    }
    void setUsage(QGLBuffer::UsagePattern p ) {
        usagePattern = p;
    }
    bool bind() {
        return (vertexBuffer->bind() && indexBuffer->bind());
    }
    void release() {
        vertexBuffer->release();
        indexBuffer->release();
    }
    void print() {
        std::cout << "GLData vertices: \n";
        //int n = 0;
        for( int n = 0; n < vertexArray.size(); ++n ) {
            std::cout << n << " : ";
            vertexArray[n].str();
            std::cout << " polys: "; 
            vertexDataArray[n].str();
            std::cout << "\n";
        }
        std::cout << "GLData polygons: \n";
        int polygonIndex = 0;
        for( int n=0; n< indexArray.size(); n=n+3) {
            std::cout << polygonIndex << " : " << indexArray[n] << " " << indexArray[n+1] << " " << indexArray[n+2] << "\n";
            ++polygonIndex;
        }
    }
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
    
    QVarLengthArray<GLVertex> vertexArray;
    QVarLengthArray<VertexData> vertexDataArray;
    QVarLengthArray<GLuint> indexArray;
    
    QGLBuffer* vertexBuffer;
    QGLBuffer* indexBuffer;
        
};

class GLWidget : public QGLWidget {
    Q_OBJECT
    public:
        GLWidget( QWidget *parent=0, char *name=0 ) ;
        ~GLWidget() {
        }
    protected:
        void initializeGL();
        void resizeGL( int width, int height );
        void paintGL();

        void keyPressEvent( QKeyEvent *e ) {
            std::cout << e->key() << " pressed.\n";
            return;
        }
        void timeOut() {
            updateGL();
        }
        
        void genVBO() {
            std::cout << " genVBO()\n";
            g.addVertex(GLVertex(-1.0f,-1.0f, 0.0f,  1.0f,0.0f,0.0f));
            g.addVertex(GLVertex( 1.0f,-1.0f, 0.0f,  0.0f,1.0f,0.0f));
            g.addVertex(GLVertex( 0.0f, 1.0f, 0.0f,  0.0f,0.0f,1.0f));
            g.addVertex(GLVertex( 1.0f, 1.0f, 0.0f,  1.0f,0.0f,1.0f));
            g.addVertex(GLVertex( -1.0f, 1.0f, 0.0f,  1.0f,1.0f,1.0f));
            g.addVertex(GLVertex( -2.0f, 0.0f, 0.0f,  1.0f,1.0f,1.0f));
            std::vector<GLuint> poly;
            poly.resize(3);
            poly[0]=0; poly[1]=1; poly[2]=2;
            g.addPolygon( poly );
            poly[0]=2; poly[1]=1; poly[2]=3;
            g.addPolygon( poly );
            poly[0]=0; poly[1]=2; poly[2]=4;
            g.addPolygon( poly );
            poly[0]=0; poly[1]=5; poly[2]=4;
            g.addPolygon( poly );
            //g.addTriangle(2,1,3);
            g.setType( GL_TRIANGLES );
            g.setUsage( QGLBuffer::StaticDraw );
            
            g.print();
            //std::cout << "removePolygon()\n";
            //g.removePolygon(0);
            std::cout << "removeVertex()\n";
            g.removeVertex(4);
            g.print();
            
            g.genVBO();
        }


    protected slots:
        void timeOutSlot() {
            timeOut();
        }
  
    private:
        GLData g;
};
#endif
