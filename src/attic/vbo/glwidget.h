#ifndef GLWIDGET
#define GLWIDGET

#include <QtOpenGL/qgl.h>
#include <QtOpenGL>
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
            
            GLfloat vertices[] = {1,1,1,  -1,1,1,  -1,-1,1,  1,-1,1,        // v0-v1-v2-v3
                      1,1,1,  1,-1,1,  1,-1,-1,  1,1,-1,        // v0-v3-v4-v5
                      1,1,1,  1,1,-1,  -1,1,-1,  -1,1,1,        // v0-v5-v6-v1
                      -1,1,1,  -1,1,-1,  -1,-1,-1,  -1,-1,1,    // v1-v6-v7-v2
                      -1,-1,-1,  1,-1,-1,  1,-1,1,  -1,-1,1,    // v7-v4-v3-v2
                      1,-1,-1,  -1,-1,-1,  -1,1,-1,  1,1,-1};   // v4-v7-v6-v5
                      
            vbuffer = new QGLBuffer(QGLBuffer::VertexBuffer);
            vbuffer->create();
            GLuint id = vbuffer->bufferId();
            std::cout << " created buffer id=" << id << "\n"; 
            vbuffer->bind();
            vbuffer->setUsagePattern(QGLBuffer::StaticDraw);
            vbuffer->allocate( vertices , sizeof(vertices) * sizeof(GLfloat));

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
// glVertexPointer(3, GL_FLOAT, 0, vertices.constData()); // THIS IS WORKING PERFECTLY glDrawElements draws elements as intended.
 
 
 
glPushMatrix();
glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, faceColor);
glDrawElements(GL_TRIANGLES,m_count, GL_UNSIGNED_INT, m_index.constData());  // AFTER CALLING THIS FUNCTION APP CRASHES ( when i use QGLBuffer.
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
        QTimer *m_timer;
        GLfloat rtri, rquad;
        MyVertex pvertex[3];

};
#endif
