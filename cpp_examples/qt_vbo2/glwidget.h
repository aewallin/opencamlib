#ifndef GLWIDGET
#define GLWIDGET

#include <QObject>
#include <QGLBuffer>
#include <QVarLengthArray>
#include <QTimer>
#include <QtGui>

#include <iostream>

#define BUFFER_OFFSET(i) ((GLbyte *)NULL + (i))

struct MyVertex {
        GLfloat x,y,z; // position
        GLfloat r,g,b; // color
        MyVertex() : x(0), y(0), z(0), r(0), g(0), b(0){};
        MyVertex(GLfloat x, GLfloat y, GLfloat z, GLfloat r, GLfloat g, GLfloat b) 
             : x(x), y(y), z(z), r(r), g(g), b(b) {};
};

class NeHeWidget : public QGLWidget {
    Q_OBJECT
    public:
        NeHeWidget( QWidget *parent=0, char *name=0 ) ;
        ~NeHeWidget() {
            vertexBuffer->destroy(); 
            indexBuffer->destroy();
        }
    protected:
        void initializeGL();
        void resizeGL( int width, int height );
        void paintGL();
        
        void paintVBO() {
            //std::cout << " paintVBO() \n";
            glPushMatrix();
            
            //if (indexBuffer->bind())
            //    std::cout << " idxBbuffer bound. " << indexBuffer->size() << "\n";
            //if (vertexBuffer->bind())
            //    std::cout << " vbuffer bound. " << vertexBuffer->size() << "\n";
            
            glLoadIdentity();
            glTranslatef(1.0f,0.0f,-6.0f);
            glEnableClientState(GL_COLOR_ARRAY);
            glEnableClientState(GL_VERTEX_ARRAY);
            //glColor3f(0.7f,0.2f,1.0f); // if no GL_COLOR_ARRAY defined, draw with only one color
            
            glVertexPointer(3, GL_FLOAT, sizeof(MyVertex), BUFFER_OFFSET(0));
            glColorPointer(3, GL_FLOAT, sizeof(MyVertex), BUFFER_OFFSET(12)); // color is offset 12-bytes from position
            
            glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0);
             
            glDisableClientState(GL_VERTEX_ARRAY);
            glDisableClientState(GL_COLOR_ARRAY);
            
            //indexBuffer->release();
            //vertexBuffer->release();
            
            glPopMatrix();
            
            // try some dummy drawing...
            glLoadIdentity();
            glTranslatef(1.0f,0.0f,-16.0f);
            glRotatef(rquad,1.0f,0.0f,0.0f);
            glColor3f(0.7f,0.2f,1.0f);
            glBegin(GL_QUADS);
                glVertex3f(-1.0f, 1.0f, 0.0f);
                glVertex3f( 1.0f, 1.0f, 0.0f);
                glVertex3f( 1.0f,-1.0f, 0.0f);
                glVertex3f(-1.0f,-1.0f, 0.0f);
            glEnd();
        }
        //void genVBO();
        void keyPressEvent( QKeyEvent *e ) {
            std::cout << e->key() << " pressed.\n";
            return;
        }
        void timeOut() {
            rtri  += 0.5; 
            rquad += 0.5;
            updateGL();
        }
        
        void genVBO() {
            std::cout << " genVBO()\n";
            mVertices.resize(3);
            mIndices.resize(3);
            mVertices[0]=MyVertex(-1.0f,-1.0f,0.0f,  1.0f,0.0f,0.0f);
            mVertices[1]=MyVertex( 1.0f,-1.0f,0.0f,  0.0f,1.0f,0.0f);
            mVertices[2]=MyVertex( 0.0f, 1.0f,0.0f,  0.0f,0.0f,1.0f);
            mIndices[0]=0;
            mIndices[1]=1;
            mIndices[2]=2;
                    
            vertexBuffer = new QGLBuffer(QGLBuffer::VertexBuffer);
            vertexBuffer->create();
            std::cout << " created buffer id=" << vertexBuffer->bufferId() << "\n"; 
            if (vertexBuffer->bind())
                std::cout << " buffer bound.\n";
            vertexBuffer->setUsagePattern(QGLBuffer::StaticDraw);
            std::cout << " allocating =" << sizeof(MyVertex)*3 << " bytes\n"; 
            vertexBuffer->allocate( mVertices.data(), sizeof(MyVertex)*3);
            std::cout << " buffer size =" << vertexBuffer->size() << "\n"; 

            indexBuffer = new QGLBuffer(QGLBuffer::IndexBuffer);
            indexBuffer->create();
            std::cout << " created idxBbuffer id=" << indexBuffer->bufferId() << "\n"; 
            if (indexBuffer->bind())
                std::cout << " idxBbuffer bound.\n";
            indexBuffer->setUsagePattern(QGLBuffer::StaticDraw);
            std::cout << " allocating =" << sizeof(GLuint)*3 << " bytes\n"; 
            indexBuffer->allocate( mIndices.data(), sizeof(GLuint)*3);
            std::cout << " idxBuffer size =" << indexBuffer->size() << "\n"; 
            std::cout << " genVBO() done\n";
        }


    protected slots:
        void timeOutSlot() {
            //std::cout << "timeOutSlot()\n";
            timeOut();
        }
  
    private:
        QGLBuffer* vertexBuffer;
        QGLBuffer* indexBuffer;
        QVarLengthArray<MyVertex,3> mVertices;
        QVarLengthArray<GLuint,3> mIndices;
        QTimer *m_timer;
        GLfloat rtri, rquad;
};
#endif
