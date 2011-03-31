#ifndef GLWIDGET
#define GLWIDGET

#include <QObject>
#include <QGLBuffer>
#include <QVarLengthArray>
#include <QtGui>
#include <QTimer>
#include <QCursor>
#include <QDataStream>

#include <boost/foreach.hpp> 
 #include <QDebug>
 
#include <iostream>
#include <cassert>
#include <set>
#include <vector>

#define PI 3.14159265

#define BUFFER_OFFSET(i) ((GLbyte *)NULL + (i))

#include "gldata.h"

template <class Scalar>
class  P3 {
public:
    P3<Scalar>() {
        x=0;
        y=0;
        z=0;
    }
    P3<Scalar>(Scalar xi, Scalar yi, Scalar zi ) {
        x=xi;
        y=yi;
        z=zi;
    }
    Scalar x;
    Scalar y;
    Scalar z;
    //friend QDataStream& operator<< ( QDataStream & stream, typename const P3<Scalar> & point );
    P3<Scalar>& operator+=( const P3<Scalar>& p) {
        x+=p.x;
        y+=p.y;
        z+=p.z;
        return *this;
    }
    //template <class Scalar>
    const P3<Scalar> operator+( const P3<Scalar> &p) const {
        return P3<Scalar>(*this) += p;
    }
    P3<Scalar>& operator-=( const P3<Scalar>& p) {
        x-=p.x;
        y-=p.y;
        z-=p.z;
        return *this;
    }
    //template <class Scalar>
    const P3<Scalar> operator-( const P3<Scalar> &p) const {
        return P3<Scalar>(*this) -= p;
    }
    const P3<Scalar>  operator*(const Scalar &a) const {
        return P3<Scalar>(*this) *= a;
    }
    P3<Scalar>& operator*=(const Scalar &a) {
        x*=a;
        y*=a;
        z*=a;
        return *this;
    }
    Scalar norm() const {
        return sqrt( x*x + y*y + z*z );
    }
    P3<Scalar> cross(const P3<Scalar> &p) const {
        Scalar xc = y * p.z - z * p.y;
        Scalar yc = z * p.x - x * p.z;
        Scalar zc = x * p.y - y * p.x;
        return P3<Scalar>(xc, yc, zc);
    }

    QString str() { return QString("(%1, %2, %3 )").arg(x).arg(y).arg(z); }

};

/*
template <class Scalar>
friend QDataStream& operator<< ( QDataStream & stream, typename const P3<Scalar> & point ) {
    stream << "( " << x " , " << y << " , " << z << " )";
    return stream;
}*/

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
        void updateDir() {
            _dirx = _up;
            P3<float> newy = _up.cross(_center-_eye);
            _diry = newy * ( (float)1.0/ newy.norm() );
            std::cout << " new diry length = " << _diry.norm() << "\n";
        }
        void panView(const QPoint& newPos) {
            std::cout << "width, height = (" << _width << " , " << _height << " )\n";
            //int idx = (newPos - oldMousePos).x(); // / 
            float dx = (float)(newPos.x() - oldMousePos.x()) / (float)_width;
            float dy = (float)(newPos.y() - oldMousePos.y()) / (float)_height;
            //int idy = (newPos - oldMousePos).y(); // / 
            //float dy = (float)idy / (float)height;
            //float dy = (float)(newPos.y() - oldPos.y()) / (float)width;
            
            // pan
            // height of window in object space: length = 2* norm(eye-center)*tan(fovy/2)
            // new position of center is:
            // center = center + dir_y *dx*length *width/height
            //                 + dir_x * dy * length
            float length = 2* (_eye-_center).norm() * tan( (_fovy/360)*2*PI /2);
            std::cout << "panning dx= "<< dx << " dy=" << dy << " length= " << length << "\n";
            P3<float> y_pan = _diry * ( dx*length* (float)_width/(float)_height );
            std::cout << " y-pan = " << y_pan.str().toStdString();
            
            _center += y_pan; // _diry * ( dx*length* (float)_width/(float)_height )  ;
            _center += _dirx * ( dy*length )  ;
            updateDir();
            // resizeGL( width(), height() );
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
        P3<float> _eye; // camera position
        P3<float> _up; // camera up-vector
        P3<float> _center; // camera look-at point
        P3<float> _diry; // rotation/pan y-axis
        P3<float> _dirx; // rotation/pan y-axis
        float _fovy; // field of vision in y-dir
        float z_near;
        float z_far;
        std::vector<GLData*> glObjects;
        //QCursor cursor;
        QPoint oldMousePos;
        int _width;
        int _height;
        bool _rightButtonPressed;
        bool _leftButtonPressed;
        QTime _lastFrameTime;
};
#endif
