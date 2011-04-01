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
    //OctreeNode n1;
    GLWidget *w = new GLWidget();
    //GLData* g = w->addObject();
    
    //GLData* rnd = w->addObject();
    //RandomSource src( rnd );
    //QObject::connect( w->timer, SIGNAL(timeout()), &src, SLOT(timeOutSlot()) );
    
    //std::cout << " genVBO()\n";
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
    /*
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
    */
    GLData* g = w->addObject();
    g->setTriangles(); 
    g->setPosition(1,0,-6);
    g->setUsageStaticDraw();
    Cutsim cs;
    cs.surf();
    std::vector<ocl::Triangle> tris = cs.getTris();
    //std::cout<
    float r=1,gr=0,b=0;
    BOOST_FOREACH( ocl::Triangle t, tris) {
        int id0 = g->addVertex( t.p[0].x, t.p[0].y, t.p[0].z, r,gr,b);
        int id1 = g->addVertex( t.p[1].x, t.p[1].y, t.p[1].z, r,gr,b);
        int id2 = g->addVertex( t.p[2].x, t.p[2].y, t.p[2].z, r,gr,b);
        // set the normals
        g->setNormal(id0, t.n.x, t.n.y, t.n.z );
        g->setNormal(id1, t.n.x, t.n.y, t.n.z );
        g->setNormal(id2, t.n.x, t.n.y, t.n.z );
        std::vector<unsigned int> poly(3);
        poly[0]=id0;
        poly[1]=id1;
        poly[2]=id2;
        g->addPolygon( poly );
    }
    
    w->show();
    return app.exec();
}
