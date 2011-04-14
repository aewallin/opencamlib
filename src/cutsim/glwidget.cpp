#include <iostream>

#include <GL/glut.h>

#include <QObject>
#include <QTimer>

#include "glwidget.h"

namespace ocl
{

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
    _up.y=0;
    _up.z=1;
    _up *= 1/_up.norm();
    _eye.x=20;
    _eye.y=20;
    _eye.z=20;
    _center.x=0;
    _center.y=0;
    _center.z=0;
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
    
    // set up lights/shading
    //set the global lighting / shading params
    glShadeModel(GL_SMOOTH); // or GL_FLAT
    //glEnable(GL_NORMALIZE); //or not (computationally expensive!)
    glEnable(GL_LIGHTING);
    //set the global ambient light
    
    
    // glLightModelf(
    // light mode is one of GL_LIGHT_MODEL_LOCAL_VIEWER,
    //                GL_LIGHT_MODEL_COLOR_CONTROL,
    //                GL_LIGHT_MODEL_TWO_SIDE
    
    // glLightModelfv
    // GL_LIGHT_MODEL_AMBIENT,
    //                GL_LIGHT_MODEL_COLOR_CONTROL,
    //                GL_LIGHT_MODEL_LOCAL_VIEWER, and
    //                GL_LIGHT_MODEL_TWO_SIDE
    
    //set the global ambient light (R, G, B, A)
    //GLfloat ambient[4]{.2,.2,.2,1};
    GLfloat ambient[4] = {.5,.5,.5,1.0};
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient);

    // set up a light:
    //GLfloat diffuseLight[] = {1,0,0,1};
    GLfloat ambientLight[] = {.5,0,0,1};
    //GLfloat specularLight[] = {1,1,1,1};
    //glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseLight);
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambientLight);
    //glLightfv(GL_LIGHT0, GL_SPECULAR, specularLight);
    glEnable(GL_LIGHT0); //enable the light
    // set last term to 0 for a spotlight (see chp 5 in ogl prog guide)
    GLfloat lightpos[] = {10,10,10,1};
    glLightfv(GL_LIGHT0,GL_POSITION, lightpos);
    
    // material property:
    
    //GLfloat specular[] = {0.6,0.6,0.6,1};

    // glMaterialf(face, paramname, value)
    // face = GL_FRONT, GL_BACK, or GL_FRONT_AND_BACK
    // GL_SHININESS
    
    // glMaterial — specify material parameters for the lighting model
    // face =         GL_AMBIENT,                rgba reflectance
    //                GL_DIFFUSE,                
    //                GL_SPECULAR,
    //                GL_EMISSION,
    //                GL_SHININESS,
    //                GL_AMBIENT_AND_DIFFUSE, or
    //                GL_COLOR_INDEXES.
    GLfloat ambientMat[] = {.5,0,0,0.5}; // rgba reflectance
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, ambientMat);
    //glMaterialfv(GL_FRONT, GL_SPECULAR, specular);


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
    //
    glMatrixMode(GL_MODELVIEW);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    gluLookAt( _eye.x, _eye.y, _eye.z, _center.x, _center.y, _center.z, _up.x, _up.y, _up.z );
    //gluLookAt( 0,0,6 , // _eye.x, _eye.y, _eye.z,
    //           0,0,0 , //_center.x, _center.y, _center.z, _up.x, _up.y, _up.z );
    //           0,1,0);
    
    
    
    //glPushMatrix();
    
    BOOST_FOREACH( GLData* g, glObjects ) { // draw each object
        //glLoadIdentity();
        //glTranslatef( g->pos.x, g->pos.y , g->pos.z ); 
        
        if ( !g->bind() )
            assert(0);
        
        glEnableClientState(GL_COLOR_ARRAY);
        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_NORMAL_ARRAY);
        
        // coords/vert, type, stride, pointer/offset
        glVertexPointer(3, GLData::coordinate_type, sizeof( GLData::vertex_type ), BUFFER_OFFSET(GLData::vertex_offset));
        glColorPointer(3, GLData::coordinate_type, sizeof( GLData::vertex_type ), BUFFER_OFFSET(GLData::color_offset)); // color is offset 12-bytes from position
        glNormalPointer( GLData::coordinate_type, sizeof( GLData::vertex_type ), BUFFER_OFFSET(GLData::normal_offset));
        
        //              mode       idx-count     type         indices*/offset
        glDrawElements( g->type , g->indexCount() , GLData::index_type, 0);
         
        glDisableClientState(GL_VERTEX_ARRAY);
        glDisableClientState(GL_COLOR_ARRAY);
        glDisableClientState(GL_NORMAL_ARRAY);
        
        g->release();
    }
    
    
    // calculate and display FPS
    int msecs =  - QTime::currentTime().msecsTo( _lastFrameTime );
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
void GLWidget::updateDir() {
    _dirx = _up;
    P3<float> newy = _up.cross(_center-_eye);
    _diry = newy * ( (float)1.0/ newy.norm() );
}
void GLWidget::zoomView( const QPoint& newPos ) {
    float dy = (float)(newPos.y() - _oldMousePos.y()) / (float)_height;
    _oldMousePos = newPos;
    zoomView( (int)(dy*120*5) );
}

void GLWidget::zoomView( int delta ) {
    float dZoom = (float)delta*(0.1/120.0);
    _eye = _center + (_eye-_center)*(dZoom + 1.0);
    updateGL();
}
void GLWidget::panView(const QPoint& newPos) {           
    float dx = (float)(newPos.x() - _oldMousePos.x()) / (float)_width;
    float dy = (float)(newPos.y() - _oldMousePos.y()) / (float)_height;
    _oldMousePos = newPos;
    float length = 2* (_eye-_center).norm() * tan( (_fovy/360)*2*PI /2);
    updateDir();
    P3<float> y_pan = _diry * ( dx*length* (float)_width/(float)_height );
    P3<float> x_pan = _dirx * ( dy*length ); 
    _center += y_pan + x_pan; 
    _eye += y_pan + x_pan;
    updateGL();
}
void GLWidget::rotateView(const QPoint& newPos) {
    float dx = (float)(newPos.x() - _oldMousePos.x()) / (float)_width;
    float dy = (float)(newPos.y() - _oldMousePos.y()) / (float)_height;
    _oldMousePos = newPos;
    updateDir();
    // rotate eye around center
    _eye.rotate(_center, _dirx, -dx * PI );
    _eye.rotate(_center, _diry, dy*PI );
    
    // now calculate the new up-vector
    P3<float> upCenter = _center + _up;
    upCenter.rotate( _center , _diry, dy*PI);
    _up = upCenter - _center;
    _up *= 1.0/_up.norm();
    
    updateGL();
}
        
} // end ocl namespace


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



