#include <QApplication>

#include <functional>
#include <boost/bind.hpp>
#include "glwidget.h"
#include "gldata.h"
#include "randomsource.h"

int OctreeNode::count =0;

int main( int argc, char **argv ) {
    QApplication a( argc, argv );
    OctreeNode n1;
    GLWidget *w = new GLWidget();
    GLData* g = w->addObject();
    
    GLData* rnd = w->addObject();
    RandomSource src( rnd );
    QObject::connect( w->timer, SIGNAL(timeout()), &src, SLOT(timeOutSlot()) );
    
    std::cout << " genVBO()\n";
    /*
    g->setTriangles(); 
    g->setPosition(1,0,-6);
    g->setUsageStaticDraw();
    g->addVertex(-1.0f,-1.0f, 0.0f,  1.0f,0.0f,0.0f, boost::bind(&OctreeNode::indexSwap, &n1, _1, _2) ); 
    g->addVertex( 1.0f,-1.0f, 0.0f,  0.0f,1.0f,0.0f, boost::bind(&OctreeNode::indexSwap, &n1, _1, _2) );
    g->addVertex( 0.0f, 1.0f, 0.0f,  0.0f,0.0f,1.0f, boost::bind(&OctreeNode::indexSwap, &n1, _1, _2) );
    g->addVertex( 1.0f, 1.0f, 0.0f,  1.0f,0.0f,1.0f, boost::bind(&OctreeNode::indexSwap, &n1, _1, _2) );
    g->addVertex( -1.0f, 1.0f, 0.0f,  1.0f,1.0f,1.0f, boost::bind(&OctreeNode::indexSwap, &n1, _1, _2) );
    g->addVertex( -2.0f, 0.0f, 0.0f,  1.0f,1.0f,1.0f, boost::bind(&OctreeNode::indexSwap, &n1, _1, _2) );
    std::vector<GLuint> poly(3);
    //poly.resize(3);
    poly[0]=0; poly[1]=1; poly[2]=2;
    g->addPolygon( poly );
    poly[0]=2; poly[1]=1; poly[2]=3;
    g->addPolygon( poly );
    poly[0]=0; poly[1]=2; poly[2]=4;
    g->addPolygon( poly );
    poly[0]=0; poly[1]=5; poly[2]=4;
    g->addPolygon( poly );
    g->print();
    //std::cout << "removePolygon()\n";
    //g.removePolygon(0);
    std::cout << "removeVertex(4)\n";
    g->removeVertex(4);
    g->print();
*/
    // now try a quad.
    OctreeNode n2;
    GLData* q = w->addObject();
    q->setQuads();
    q->setPosition(2,0,-6);
    q->setUsageStaticDraw(); 
    q->addVertex(-3.0f,0.0f,0.0f,0.0f,0.0f,1.0f, boost::bind(&OctreeNode::indexSwap, &n2, _1, _2));
    q->addVertex(-3.0f,1.0f,0.0f,0.0f,0.0f,1.0f, boost::bind(&OctreeNode::indexSwap, &n2, _1, _2));
    q->addVertex(-4.0f,1.0f,0.0f,0.0f,0.0f,1.0f, boost::bind(&OctreeNode::indexSwap, &n2, _1, _2));
    q->addVertex(-4.0f,0.0f,0.0f,0.0f,0.0f,1.0f, boost::bind(&OctreeNode::indexSwap, &n2, _1, _2));
    std::vector<GLuint> quad(4);
    quad[0]=0; quad[1]=1; quad[2]=2; quad[3]=3;
    q->addPolygon(quad);
    
    q->print();
    std::cout << "Q removeVertex(3)\n";
    //q->removeVertex(0);
    q->print();
    
    w->show();
    return a.exec();
}
