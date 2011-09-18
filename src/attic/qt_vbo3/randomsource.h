
#ifndef RS_H
#define RS_H

#include <QObject>
//#include <QGLBuffer>
//#include <QVarLengthArray>

#include "gldata.h"

//#include <boost/function.hpp>

// test class which generates random points.
class RandomSource : public QObject {
    Q_OBJECT
public:
    RandomSource(GLData* g) {
        _g = g;
        _g->setPoints(); 
        _g->setPosition(1,0,-6);
        _g->setUsageDynamicDraw();
    }
    ~RandomSource() {
    }
    void addRandomPoint () {
        float x,y,z,r,g,b;
        x= (float)qrand() / (float)RAND_MAX;
        y= (float)qrand() / (float)RAND_MAX;
        z=0;
        r= (float)qrand() / (float)RAND_MAX;
        g= (float)qrand() / (float)RAND_MAX;
        b= (float)qrand() / (float)RAND_MAX;
        GLVertex v(x,y,z,r,g,b);
        std::cout << " adding "; 
        v.str();
        std::cout << "\n"; 
        unsigned id = _g->addVertex(v);
        std::vector<unsigned> verts(1);
        verts[0]=id;
        _g->addPolygon(verts);
        _g->updateVBO();
        //_g->print();
    };
public slots:
  
    void timeOutSlot() {
        //std::cout << " src::timeout() ! \n";
        addRandomPoint();
    }
private:
    GLData* _g;
};

#endif
