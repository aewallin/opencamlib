

#include <iostream>
#include <QObject>

#include "nehewidget.h"

NeHeWidget::NeHeWidget( QWidget *parent, char *name ) 
  : QGLWidget(parent) {
    //QGLFormat(QGL::SampleBuffers), parent) {
    m_timer = new QTimer(this);
    m_timer->setInterval(10);
    
    connect( m_timer, SIGNAL(timeout()), 
                      this, SLOT(timeOutSlot()) );

    rtri = 0.0;
    rquad = 0.0;
    m_timer->start();
  }

NeHeWidget::~NeHeWidget() {
}

void NeHeWidget::initializeGL() {
    std::cout << "initializeGL()\n";
    glShadeModel(GL_SMOOTH);
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    glLoadIdentity();
    
    // glEnable(GL_DEPTH_TEST);

}


void NeHeWidget::resizeGL( int width, int height ) {
    std::cout << "resizeGL(" << width << " , " << height << " )\n";
    if (height == 0)    {
       height = 1;
    }

    // Reset The Current Viewport
    glViewport(0, 0, width, height);

    // Select The Projection Matrix
    glMatrixMode(GL_PROJECTION);

    // Reset The Projection Matrix
    glLoadIdentity();

    // Calculate The Aspect Ratio Of The Window
    gluPerspective(45.0f, (GLfloat)width / (GLfloat)height,
       0.1f, 100.0f);

    // Select The Modelview Matrix
    glMatrixMode(GL_MODELVIEW);

    // Reset The Modelview Matrix
    glLoadIdentity();
    return;
}

void NeHeWidget::paintGL()  {
    //std::cout << "paintGL()\n";
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glLoadIdentity();
  glTranslatef(-1.5f,0.0f,-6.0f);
  glRotatef(rtri,0.0f,1.0f,0.0f);
  glColor3f(1.0f,0.0f,0.0f);
  glBegin(GL_TRIANGLES);
    glColor3f(1.0f,0.0f,0.0f);
    glVertex3f( 0.0f, 1.0f, 0.0f);
    glColor3f(0.0f,1.0f,0.0f);
    glVertex3f(-1.0f,-1.0f, 0.0f);
    glColor3f(0.0f,0.0f,1.0f);
    glVertex3f( 1.0f,-1.0f, 0.0f);
  glEnd();
  glLoadIdentity();
  glTranslatef(1.5f,0.0f,-6.0f);
  glRotatef(rquad,1.0f,0.0f,0.0f);
  glColor3f(0.5f,0.5f,1.0f);
  glBegin(GL_QUADS);
    glVertex3f(-1.0f, 1.0f, 0.0f);
    glVertex3f( 1.0f, 1.0f, 0.0f);
    glVertex3f( 1.0f,-1.0f, 0.0f);
    glVertex3f(-1.0f,-1.0f, 0.0f);
  glEnd();
}
