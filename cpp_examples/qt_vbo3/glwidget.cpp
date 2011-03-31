#include <iostream>

#include <GL/glut.h>

#include <QObject>
#include <QTimer>

#include "glwidget.h"

GLWidget::GLWidget( QWidget *parent, char *name ) 
  : QGLWidget(parent) {
    timer = new QTimer(this);
    timer->setInterval(1000);
    connect( timer, SIGNAL(timeout()), this, SLOT(timeOutSlot()) );
    timer->start();
    _fovy = 60.0;
    z_near = 0.1;
    z_far = 100.0;
    _up.x=0;
    _up.y=1;
    _up.z=0;
    _up *= 1/_up.norm();
    _eye.x=0;
    _eye.y=0;
    _eye.z=0;
    _center.x=0;
    _center.y=0;
    _center.z=-10;
    //setCursor(cursor);
    updateDir();
}

GLData* GLWidget::addObject() {
    GLData* g = new GLData();
    glObjects.push_back(g);
    return g;
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
    if (height == 0)    {
       height = 1;
    }
    _width = width;
    _height = height;
    std::cout << "resizeGL(" << width << " , " << height << " )\n";
    
    glViewport(0, 0, _width, _height); // Reset The Current Viewport
    glMatrixMode(GL_PROJECTION); // Select The Projection Matrix
    glLoadIdentity(); // Reset The Projection Matrix
    
    // Calculate The Aspect Ratio Of The Window
    // void gluPerspective( fovy, aspect, zNear, zFar);
    gluPerspective( _fovy, (GLfloat)_width / (GLfloat)_height, z_near, z_far);
    
    glMatrixMode(GL_MODELVIEW); // Select The Modelview Matrix
    glLoadIdentity();
    gluLookAt( _eye.x, _eye.y, _eye.z, _center.x, _center.y, _center.z, _up.x, _up.y, _up.z );
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //glLoadIdentity(); // Reset The Modelview Matrix
    return;
}

void GLWidget::paintGL()  {
    //glMatrixMode(GL_PROJECTION); 
    //glLoadIdentity();
    //gluLookAt( _eye.x, _eye.y, _eye.z, _center.x, _center.y, _center.z, _up.x, _up.y, _up.z );
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    
    //glPushMatrix();
    
    BOOST_FOREACH( GLData* g, glObjects ) { // draw each object
        glLoadIdentity();
        glTranslatef( g->pos.x, g->pos.y , g->pos.z ); 
        
        if ( !g->bind() )
            assert(0);
        
        glEnableClientState(GL_COLOR_ARRAY);
        glEnableClientState(GL_VERTEX_ARRAY);
        
        // coords/vert, type, stride, pointer/offset
        glVertexPointer(3, GLData::coordinate_type, sizeof( GLData::vertex_type ), BUFFER_OFFSET(GLData::vertex_offset));
        glColorPointer(3, GLData::coordinate_type, sizeof( GLData::vertex_type ), BUFFER_OFFSET(GLData::color_offset)); // color is offset 12-bytes from position
        
        //              mode       idx-count     type         indices*/offset
        glDrawElements( g->type , g->indexCount() , GLData::index_type, 0);
         
        glDisableClientState(GL_VERTEX_ARRAY);
        glDisableClientState(GL_COLOR_ARRAY);
        
        g->release();
    }
    
    
    // calculate and display FPS
    int msecs =  - _lastFrameTime.msecsTo( _lastFrameTime );
    if (msecs == 0)
        msecs = 1;
    float fps = (float)1000/(float)msecs;
    QString fps_str;
    fps_str = QString("%1 FPS, eye=%2, center=%3, up=%4").arg(fps).arg(_eye.str()).arg(_center.str()).arg(_up.str());
    renderText( 10, 20, fps_str );
    _lastFrameTime = QTime::currentTime();
    
    QString dir_str;
    dir_str = QString("dir x = %1 dir y = %2 norm(y)= %3 ").arg(_dirx.str()).arg(_diry.str()).arg(_diry.norm());
    renderText( 10, 50, dir_str );
    
}


// mouse movement
// dx in [-1,1]
// dx = (xnew - xold) / width
// dy = (ynew - yold) / height

// function that rotates point around axis:
// rotatePoint( point, origin, direction, angle)

// rotation around axis: 
// dir_x = up
// dir_y = up cross (center-eye) / norm( up cross (center-eye) )
// now rotations are:
// eye = rotatePoint(eye, center, dir_x, -dx * pi )
// eye = rotatePoint(eye, center, dir_y, dy*pi )
// up = rotatePoint( center+up, center, dir_y, dy*pi) - center
// (normalize up after operations)
//
// zoom
// eye = center + (eye-center)*(dy+1)
//
// pan
// height of window in object space: length = 2* norm(eye-center)*tan(fovy/2)
// new position of center is:
// center = center + dir_y *dx*length *width/height
//                 + dir_x * dy * length



