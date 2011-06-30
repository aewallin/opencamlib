#include <QApplication>

#include <functional>
#include <boost/bind.hpp>

#include <opencamlib/gldata.h>
#include <opencamlib/glwidget.h>


#include "cutsim.h"


int main( int argc, char **argv ) {
    QApplication app( argc, argv );
    
    // create a new GLWidget, which inherits from QGLWidget. This is the OpenGL view.
    // it displays GLData objects.
    ocl::GLWidget *w = new ocl::GLWidget();
    
    // create and return a new GLData object for us
    ocl::GLData* g = w->addObject();
    Cutsim cs;
    // QObject::connect( w, SIGNAL(sig()), &cs, SLOT(cut()) );
    
    // link GLData with cutting-simulation
    cs.setGLData(g);
    
    // update GL-data
    cs.updateGL();
    
    // show the main window
    w->show();
    return app.exec();
}
