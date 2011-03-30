#ifndef GLWIDGET
#define GLWIDGET

#include <QObject>
#include <QGLBuffer>
#include <QVarLengthArray>
#include <QtGui>
#include <QTimer>
#include <QCursor>

#include <boost/foreach.hpp> 
 #include <QDebug>
 
#include <iostream>
#include <cassert>
#include <set>
#include <vector>

#define BUFFER_OFFSET(i) ((GLbyte *)NULL + (i))

#include "gldata.h"

template <class Scalar>
struct P3 {
    Scalar x;
    Scalar y;
    Scalar z;
};

class GLWidget : public QGLWidget {
    Q_OBJECT
    public:
        GLWidget( QWidget *parent=0, char *name=0 ) ;
        ~GLWidget() {}

        void initializeGL();
        /// add new GLData object and return pointer to it.

        GLData* addObject();
        QTimer* timer;
    protected:
        
        void resizeGL( int width, int height );
        void paintGL();
        void panView(const QPoint& newPos) {
            //int idx = (newPos - oldMousePos).x(); // / 
            float dx = (float)(newPos.x() - oldMousePos.x()) / (float)_width;
            float dy = (float)(newPos.y() - oldMousePos.y()) / (float)_height;
            //int idy = (newPos - oldMousePos).y(); // / 
            //float dy = (float)idy / (float)height;
            //float dy = (float)(newPos.y() - oldPos.y()) / (float)width;
            std::cout << "panning dx= "<< dx << " dy=" << dy << "\n";
        }
        
        void keyPressEvent( QKeyEvent *e ) {
            std::cout << e->key() << " pressed.\n";
            return;
        }
        void mouseMoveEvent( QMouseEvent *e ) {
            qDebug() << " mouseMove : " << e->pos() << " button=" << e->button() << "\n";
            if (_leftButtonPressed ) {
                panView( e->pos() ); 
            }
        }
        void mousePressEvent( QMouseEvent *e ) {
            qDebug() << " mousePress : " << e->pos() << " button=" << e->button() << "\n";
            // button = 1  is left
            // button = 2  is right
            // button = 4  is middle 
            oldMousePos = e->pos();
            if (e->button() == Qt::LeftButton) {
                setCursor(Qt::OpenHandCursor);
                _leftButtonPressed = true;
                std::cout << " left button press\n";
            } else if (e->button() == Qt::RightButton) {
                setCursor(Qt::SizeAllCursor);
                _rightButtonPressed = true;
            }
        }
        void mouseReleaseEvent( QMouseEvent *e ) {
            qDebug() << " mouseRelease : " << e->pos() << "\n";
            setCursor( Qt::ArrowCursor);
            _rightButtonPressed = false;
            _leftButtonPressed = false;
        }
        void timeOut() {
            updateGL();
        }
        
        void genVBO() {
            BOOST_FOREACH(GLData* g, glObjects) {
                g->genVBO();
            }
        }


    protected slots:
        void timeOutSlot() {
            timeOut();
        }
  
    private:
        P3<float> eye; // camera position
        P3<float>  up; // camera up-vector
        P3<float> center; // camera look-at point
        float fov_y; // field of vision in y-dir
        float z_near;
        float z_far;
        std::vector<GLData*> glObjects;
        //QCursor cursor;
        QPoint oldMousePos;
        int _width;
        int _height;
        bool _rightButtonPressed;
        bool _leftButtonPressed;
};
#endif
