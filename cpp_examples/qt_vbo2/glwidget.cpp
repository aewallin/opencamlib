#include <iostream>

#include <GL/glut.h>

#include <QObject>
#include <QTimer>

#include "glwidget.h"

GLWidget::GLWidget( QWidget *parent, char *name ) 
  : QGLWidget(parent) {
    timer = new QTimer(this);
    timer->setInterval(10);
    connect( timer, SIGNAL(timeout()), this, SLOT(timeOutSlot()) );

    timer->start();

}


void GLWidget::initializeGL() {
    std::cout << "initializeGL()\n";
    glShadeModel(GL_SMOOTH);
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    glLoadIdentity();
    // glEnable(GL_DEPTH_TEST);
    genVBO();  
}


void GLWidget::resizeGL( int width, int height ) {
    std::cout << "resizeGL(" << width << " , " << height << " )\n";
    if (height == 0)    {
       height = 1;
    }
    glViewport(0, 0, width, height); // Reset The Current Viewport
    glMatrixMode(GL_PROJECTION); // Select The Projection Matrix
    glLoadIdentity(); // Reset The Projection Matrix
    
    // Calculate The Aspect Ratio Of The Window
    // void gluPerspective( fovy, aspect, zNear, zFar);
    gluPerspective(45.0f, (GLfloat)width / (GLfloat)height, 0.1f, 100.0f);
    
    glMatrixMode(GL_MODELVIEW); // Select The Modelview Matrix
    glLoadIdentity(); // Reset The Modelview Matrix
    return;
}



void GLWidget::paintGL()  {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glPushMatrix();
    
    BOOST_FOREACH( GLData* g, glObjects ) { // draw each object
        glLoadIdentity();
        glTranslatef( g->pos.x, g->pos.y , g->pos.z ); 
        
        if ( !g->bind() )
            assert(0);
        
        glEnableClientState(GL_COLOR_ARRAY);
        glEnableClientState(GL_VERTEX_ARRAY);
        //glColor3f(0.7f,0.2f,1.0f); // if no GL_COLOR_ARRAY defined, draw with only one color
        
        // coords/vert, type, stride, pointer/offset
        glVertexPointer(3, GLData::coordinate_type, sizeof( GLData::vertex_type ), BUFFER_OFFSET(GLData::vertex_offset));
        glColorPointer(3, GLData::coordinate_type, sizeof( GLData::vertex_type ), BUFFER_OFFSET(GLData::color_offset)); // color is offset 12-bytes from position
        
        //              mode       idx-count     type         indices*/offset
        glDrawElements( g->type , g->indexCount() , GLData::index_type, 0);
         
        glDisableClientState(GL_VERTEX_ARRAY);
        glDisableClientState(GL_COLOR_ARRAY);
        
        g->release();
    }
    glPopMatrix();

}
