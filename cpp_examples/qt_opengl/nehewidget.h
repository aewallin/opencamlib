#ifndef NEHEWIDGET
#define NEHEWIDGET

#include <QtOpenGL/qgl.h>
#include <QTimer>
#include <QtGui>
#include <iostream>

class NeHeWidget : public QGLWidget
{
  Q_OBJECT
public:
  NeHeWidget( QWidget *parent=0, char *name=0 ) ;
  ~NeHeWidget();
protected:
  void initializeGL();
  void resizeGL( int width, int height );
  void paintGL();
  void keyPressEvent( QKeyEvent *e ) {return;}
  void timeOut() {rtri +=0.5; rquad+=0.5;updateGL();}
  
protected slots:
    void timeOutSlot() {
        //std::cout << "timeOutSlot()\n";
        timeOut();
    }
  
private:
  QTimer *m_timer;
  GLfloat rtri, rquad;
};
#endif
