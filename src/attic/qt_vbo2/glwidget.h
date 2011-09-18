#ifndef GLWIDGET
#define GLWIDGET

#include <QObject>
#include <QGLBuffer>
#include <QVarLengthArray>
#include <QtGui>
#include <QTimer>

#include <boost/foreach.hpp> 

#include <iostream>
#include <cassert>
#include <set>
#include <vector>

#define BUFFER_OFFSET(i) ((GLbyte *)NULL + (i))

#include "gldata.h"


class GLWidget : public QGLWidget {
    Q_OBJECT
    public:
        GLWidget( QWidget *parent=0, char *name=0 ) ;
        ~GLWidget() {}

        void initializeGL();
        /// add new GLData object and return pointer to it.

        GLData* addObject();
        
    protected:
        
        void resizeGL( int width, int height );
        void paintGL();

        void keyPressEvent( QKeyEvent *e ) {
            std::cout << e->key() << " pressed.\n";
            return;
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
        std::vector<GLData*> glObjects;
        QTimer* timer;
};
#endif
