/*  $Id$
 * 
 *  Copyright 2010-2011 Anders Wallin (anders.e.e.wallin "at" gmail.com)
 *  
 *  This file is part of OpenCAMlib.
 *
 *  OpenCAMlib is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  OpenCAMlib is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with OpenCAMlib.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef GLWIDGET_H
#define GLWIDGET_H

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

#include "gldata.h"
#include "p3.h"

namespace ocl
{

#define PI 3.14159265

#define BUFFER_OFFSET(i) ((GLbyte *)NULL + (i))



class GLWidget : public QGLWidget {
    Q_OBJECT
    public:
        GLWidget( QWidget *parent=0, char *name=0 ) ;
        ~GLWidget() {}

        void initializeGL();
        /// add new GLData object and return pointer to it.

        GLData* addObject();
        QTimer* timer;
    signals:
        void sig();

    protected:
        
        void resizeGL( int width, int height );
        void paintGL();
        void updateDir();
        void zoomView( int delta );
        void zoomView( const QPoint& newPos );
        void panView(const QPoint& newPos);
        void rotateView(const QPoint& newPos);
        void keyPressEvent( QKeyEvent *e ) {
            std::cout << e->key() << " pressed.\n";
            if ( e->key() == Qt::Key_C )
                emit sig();
            return;
        }
        void mouseMoveEvent( QMouseEvent *e ) {
            if (_leftButtonPressed ) {
               panView( e->pos() ); 
            } else if (_rightButtonPressed) {
                rotateView( e->pos() ); 
            }
            else if (_middleButtonPressed) {
                zoomView( e->pos() ); 
            }
        }
        void wheelEvent( QWheelEvent *e ) {
            //qDebug() << " mouseWheel delta= " << e->delta() << "\n";
            zoomView( e->delta() );
        }
        void mousePressEvent( QMouseEvent *e ) {
            //qDebug() << " mousePress : " << e->pos() << " button=" << e->button() << "\n";
            _oldMousePos = e->pos();
            if (e->button() == Qt::LeftButton) {
                setCursor(Qt::OpenHandCursor);
                _leftButtonPressed = true;
                //std::cout << " left button press\n";
            } else if (e->button() == Qt::RightButton) {
                setCursor(Qt::SizeAllCursor);
                _rightButtonPressed = true;
            }
            else if (e->button() == Qt::MiddleButton) {
                setCursor(Qt::SplitVCursor);
                _middleButtonPressed = true;
            }
        }
        void mouseReleaseEvent( QMouseEvent *e ) {
            //qDebug() << " mouseRelease : " << e->pos() << "\n";
            setCursor( Qt::ArrowCursor);
            _rightButtonPressed = false;
            _leftButtonPressed = false;
            _middleButtonPressed = false;
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
        void timeOutSlot() { timeOut(); }
  
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

        QPoint _oldMousePos;
        int _width;
        int _height;
        bool _rightButtonPressed;
        bool _leftButtonPressed;
        bool _middleButtonPressed;
        QTime _lastFrameTime;
};

} // end ocl namespace

#endif
