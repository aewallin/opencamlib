#include <QApplication>

#include <functional>
#include <boost/bind.hpp>
#include "glwidget.h"
#include "gldata.h"
//#include "randomsource.h"

#include "cutsim.h"

//int OctreeNode::count =0;

int main( int argc, char **argv ) {
    QApplication app( argc, argv );
    
    GLWidget *w = new GLWidget();
    
    GLData* g = w->addObject();
    Cutsim cs;
    QObject::connect( w, SIGNAL(sig()), &cs, SLOT(cut()) );
    
    cs.setGLData(g);
    cs.updateGL();
    
    w->show();
    return app.exec();
}
