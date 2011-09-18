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

    void rotate(const P3<Scalar>& origin, const P3<Scalar>& v, Scalar alfa) {
        // rotate point p by alfa deg/rad around vector o->v
        // p = o + M*(p-o)
        Scalar M[3][3];
        Scalar c = cos(alfa);
        Scalar D = 1.0 - c;
        Scalar s = sin(alfa);
        M[0][0] = v.x*v.x*D+c; 
        M[0][1] = v.y*v.x*D+v.z*s; 
        M[0][2] = v.z*v.x*D-v.y*s;
        M[1][0] = v.x*v.y*D-v.z*s;
        M[1][1] = v.y*v.y*D+c;
        M[1][2] = v.z*v.y*D+v.x*s;
        M[2][0] = v.x*v.z*D+v.y*s;
        M[2][1] = v.y*v.z*D-v.x*s;
        M[2][2] = v.z*v.z*D+c;
        // matrix multiply
        Scalar vector[3];
        vector[0] = x - origin.x;
        vector[1] = y - origin.y;
        vector[2] = z - origin.z;
        Scalar result[3];
        for (int i=0; i < 3; i++) {
            result[i]=0;
            for (int j=0; j < 3; j++)
	            result[i]+=vector[j]*M[i][j];
        }
        x = origin.x + result[0];
        y = origin.y + result[1];
        z = origin.z + result[2];
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
        }
        void zoomView( int delta ) {
            float dZoom = (float)delta*(0.1/120.0);
            _eye = _center + (_eye-_center)*(dZoom + 1.0);
            updateGL();
        }
        void panView(const QPoint& newPos) {           
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
        void rotateView(const QPoint& newPos) {
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
        void keyPressEvent( QKeyEvent *e ) {
            std::cout << e->key() << " pressed.\n";
            return;
        }
        void mouseMoveEvent( QMouseEvent *e ) {
            if (_leftButtonPressed ) {
               panView( e->pos() ); 
            } else if (_rightButtonPressed) {
                rotateView( e->pos() ); 
            }
        }
        void wheelEvent( QWheelEvent *e ) {
            qDebug() << " mouseWheel delta= " << e->delta() << "\n";
            zoomView( e->delta() );
        }
        void mousePressEvent( QMouseEvent *e ) {
            qDebug() << " mousePress : " << e->pos() << " button=" << e->button() << "\n";
            _oldMousePos = e->pos();
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
        QPoint _oldMousePos;
        int _width;
        int _height;
        bool _rightButtonPressed;
        bool _leftButtonPressed;
        QTime _lastFrameTime;
};
#endif
