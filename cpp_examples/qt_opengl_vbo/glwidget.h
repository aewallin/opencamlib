#ifndef GLWIDGET
#define GLWIDGET

//#include <QtOpenGL/qgl.h>
//#include <QtOpenGL>
#include <QObject>
#include <QGLBuffer> // only in Qt 4.7 !
//#include <QtOpenGL/glew.h>
//GL/glew.h
#include <QTimer>
#include <QtGui>
#include <iostream>

struct MyVertex
 {
   float x, y, z;        //Vertex
   float nx, ny, nz;     //Normal
   float s0, t0;         //Texcoord0
 };


class NeHeWidget : public QGLWidget {
    Q_OBJECT
    public:
        NeHeWidget( QWidget *parent=0, char *name=0 ) ;
        ~NeHeWidget() {vbuffer->destroy(); }
    protected:
        void initializeGL();
        void resizeGL( int width, int height );
        void paintGL();
        
        void paintVBO() {
            std::cout << " paintVBO() \n";
            //((glPushMatrix();
            
            if (idxBuffer->bind())
                std::cout << " idxBbuffer bound. "<<  idxBuffer->size() << "\n";
            if (vbuffer->bind())
                std::cout << " vbuffer bound. "<<  vbuffer->size() << "\n";
            
            glLoadIdentity();
            //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            //glTranslatef(1.0f,0.0f,-6.0f);
            glColor3f(1.0f,0.0f,0.0f);
            //glTranslatef(-1.5f,0.0f,-6.0f);
            glEnableClientState(GL_VERTEX_ARRAY);
            glEnableClientState(GL_INDEX_ARRAY);
            glVertexPointer(3, GL_FLOAT, 0, 0); // size, type, stride, pointer
            glIndexPointer(GL_INT, 0,0); // size, type, stride, pointer
            glDrawElements(GL_QUADS, 24, GL_INT, 0);  // mode, count, type, indices-pointer
            //glDrawArrays(GL_QUADS, 0, 24);
            
            glDisableClientState(GL_VERTEX_ARRAY);
            glDisableClientState(GL_INDEX_ARRAY);
            idxBuffer->release();
            vbuffer->release();
            
            //glPopMatrix();
            
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
            
            GLfloat vertices[] = {
                      1,1,0,  -1,1,-8,  -1,-1,0,  1,-1,-8,        // v0-v1-v2-v3
                      1,1,0,  1,-1,1,  1,-1,0,  -8,1,-1,        // v0-v3-v4-v5
                      1,1,0,  1,1,-1,  -1,1,0,  -1,1,1,        // v0-v5-v6-v1
                      -1,-8,0,  -1,1,0,  -1,-1,-1,  -1,-1,1,    // v1-v6-v7-v2
                      -1,-1,0,  1,-1,0,  1,-1,8,  -1,-1,1,    // v7-v4-v3-v2
                      1,-1,0,  -8,-1,0,  -1,1,-1,  -8,1,-1};   // v4-v7-v6-v5
                      
            vbuffer = new QGLBuffer(QGLBuffer::VertexBuffer);
            vbuffer->create();
            //GLuint id = vbuffer->bufferId();
            std::cout << " created buffer id=" << vbuffer->bufferId() << "\n"; 
            if (vbuffer->bind())
                std::cout << " buffer bound.\n";
            vbuffer->setUsagePattern(QGLBuffer::StaticDraw);
            std::cout << " allocating =" << sizeof(vertices) * sizeof(GLfloat) << " bytes\n"; 
            vbuffer->allocate( vertices , sizeof(vertices) * sizeof(GLfloat));
            std::cout << " buffer size =" << vbuffer->size() << "\n"; 
            
            GLint idx[] = {0,1,2,3,         // v0-v1-v2-v3
                      4,5,6,7,
                      8,9,10,11,
                      12,13,14,15,
                      16,17,18,19,
                      20,21,22,23};
            
            idxBuffer = new QGLBuffer(QGLBuffer::IndexBuffer);
            idxBuffer->create();
            //GLuint id = vbuffer->bufferId();
            std::cout << " created idxBbuffer id=" << idxBuffer->bufferId() << "\n"; 
            if (idxBuffer->bind())
                std::cout << " idxBbuffer bound.\n";
            idxBuffer->setUsagePattern(QGLBuffer::StaticDraw);
            std::cout << " allocating =" << sizeof(idx) * sizeof(GLint) << " bytes\n"; 
            idxBuffer->allocate( idx , sizeof(idx) * sizeof(GLint));
            std::cout << " idxBuffer size =" << idxBuffer->size() << "\n"; 
            
            idxBuffer->release();
            vbuffer->release();
            //GLuint vboId;  
            ///GLuint vCount=3;   
            //GLfloat* vertices = new GLfloat[vCount*3];
            //glGenBuffersARB(1, vboId); // create buffer
            //glBindBufferARB(GL_ARRAY_BUFFER_ARB, vboId); // use buffer
            // upload data
            //glBufferDataARB(GL_ARRAY_BUFFER_ARB, sizeof(MyVertex)*3, &pvertex[0].x, GL_STATIC_DRAW_ARB);
            //delete [] vertices;
            
        }
        /*
         * glEnableClientState(GL_VERTEX_ARRAY);
 
 
glVertexPointer(3, GL_FLOAT, 0, 0);    // THIS IS NOT WORKING - glDrawElements cause app to crash.
// glVertexPointer(3, GL_FLOAT, 0, vertices.constData()); 
* // THIS IS WORKING PERFECTLY glDrawElements draws elements as intended.
 
 
 
glPushMatrix();
glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, faceColor);
glDrawElements(GL_TRIANGLES,m_count, GL_UNSIGNED_INT, m_index.constData());  
* // AFTER CALLING THIS FUNCTION APP CRASHES ( when i use QGLBuffer.

//checkGLError();
glDisableClientState(GL_VERTEX_ARRAY);
glPopMatrix();
* 
         * */

    protected slots:
        void timeOutSlot() {
            //std::cout << "timeOutSlot()\n";
            timeOut();
        }
  
    private:
        QGLBuffer* vbuffer;
        QGLBuffer* idxBuffer;
        QTimer *m_timer;
        GLfloat rtri, rquad;
        MyVertex pvertex[3];

};
#endif
