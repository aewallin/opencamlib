#include <QApplication>



#include <opencamlib/glwidget.h>
#include <opencamlib/gldata.h>


#include "cutsim.h"


int main( int argc, char **argv ) {
    QApplication app( argc, argv );
    
    ocl::GLWidget *w = new ocl::GLWidget();
    
    ocl::GLData* g = w->addObject();
    Cutsim cs;
    QObject::connect( w, SIGNAL(sig()), &cs, SLOT(cut()) );
    
    cs.setGLData(g);
    cs.updateGL();
    
    w->show();
    return app.exec();
}
