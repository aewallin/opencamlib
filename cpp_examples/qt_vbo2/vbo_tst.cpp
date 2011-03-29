#include <QApplication>
#include "glwidget.h"
#include "gldata.h"

int main( int argc, char **argv )
{
    QApplication a( argc, argv );
    GLWidget *w = new GLWidget();
    GLData* g = w->addObject();
    std::cout << " genVBO()\n";
    g->setTriangles(); 
    g->setUsage( QGLBuffer::StaticDraw );
    g->addVertex(GLVertex(-1.0f,-1.0f, 0.0f,  1.0f,0.0f,0.0f));
    g->addVertex(GLVertex( 1.0f,-1.0f, 0.0f,  0.0f,1.0f,0.0f));
    g->addVertex(GLVertex( 0.0f, 1.0f, 0.0f,  0.0f,0.0f,1.0f));
    g->addVertex(GLVertex( 1.0f, 1.0f, 0.0f,  1.0f,0.0f,1.0f));
    g->addVertex(GLVertex( -1.0f, 1.0f, 0.0f,  1.0f,1.0f,1.0f));
    g->addVertex(GLVertex( -2.0f, 0.0f, 0.0f,  1.0f,1.0f,1.0f));
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
    //std::cout << "removeVertex()\n";
    g->removeVertex(4);
    g->print();

    // now try a quad.
    GLData* q = w->addObject();
    q->setQuads();
    q->setUsage( QGLBuffer::StaticDraw );
    q->addVertex(-3.0f,0.0f,0.0f,0.0f,0.0f,1.0f);
    q->addVertex(-3.0f,1.0f,0.0f,0.0f,0.0f,1.0f);
    q->addVertex(-4.0f,1.0f,0.0f,0.0f,0.0f,1.0f);
    q->addVertex(-4.0f,0.0f,0.0f,0.0f,0.0f,1.0f);
    std::vector<GLuint> quad(4);
    quad[0]=0; quad[1]=1; quad[2]=2; quad[3]=3;
    q->addPolygon(quad);
    q->print();
    
    w->show();
    return a.exec();
}
