#ifndef CUTSIM_H
#define CUTSIM
#include <QObject>

#include <string>
#include <iostream>
#include <cmath>
#include <vector>

//#include <functional>
#include <boost/bind.hpp>

#include <opencamlib/point.h>
#include <opencamlib/triangle.h>
#include <opencamlib/numeric.h>
#include <opencamlib/octree.h>
#include <opencamlib/octnode.h>

#include <opencamlib/volume.h>
#include <opencamlib/marching_cubes.h>

#include <opencamlib/gldata.h>

/// a Cutsim stores an Octree stock model, uses an iso-surface extraction
/// algorithm to generate surface triangles, and communicates with
/// the corresponding GLData surface which is used for rendering
class Cutsim : public QObject {
    Q_OBJECT

public:
    // std::cout << ocl::revision() << "\n";
    // std::cout << " Experimental C++ cutting simulation.\n";
    // Octree(root_scale, max_depth, cp)
    Cutsim () {
        ocl::Point octree_center(0,0,0);
        unsigned int max_depth = 7;
        tree = new ocl::Octree(10.0, max_depth, octree_center );
        std::cout << " tree before init: " << tree->str() << "\n";
        tree->init(2u);
        tree->debug=false;
        std::cout << " tree after init: " << tree->str() << "\n";
        
        ocl::SphereOCTVolume stock_sphere;
        stock_sphere.radius = 7;
        stock_sphere.center = ocl::Point(0,0,0);
        stock_sphere.calcBB();
        stock_sphere.invert = true;
        
        tree->diff_negative( &stock_sphere );
        
        std::cout << " tree after pane-cut: " << tree->str() << "\n";
        
        /*
        ocl::SphereOCTVolume s;
        s.radius = 2;
        s.center = ocl::Point(4,4,4);
        s.calcBB();
        std::cout << " before diff: " << tree->str() << "\n";
        tree->diff_negative( &s );
        std::cout << " AFTER diff: " << tree->str() << "\n";
        */
        
        mc = new ocl::MarchingCubes();
        tree->setIsoSurf(mc);
        //tree->debug=true;
    } 
    void setGLData(ocl::GLData* gldata) {
        // this is the GLData that corresponds to the tree
        g = gldata;
        g->setTriangles(); // mc: triangles, dual_contour: quads
        g->setPosition(0,0,0); // position offset (?used)
        g->setUsageDynamicDraw();
        tree->setGLData(g);
    }
    void updateGL() {
        // traverse the octree and update the GLData correspondingly
        //ocl::Octnode* root = tree->getRoot();
        tree->updateGL();
    }
    

    

    
public slots:
    void cut() { // demo slot of doing a cutting operation on the tree with a volume.
        std::cout << " cut! called \n";
        ocl::SphereOCTVolume s;
        s.radius = 2;
        s.center = ocl::Point(4,4,4);
        s.calcBB();
        //std::cout << " before diff: " << tree->str() << "\n";
        tree->diff_negative( &s );
        g->updateVBO();
        //std::cout << " AFTER diff: " << tree->str() << "\n";

        updateGL();
        g->updateVBO();
    }
    
private:
    ocl::MarchingCubes* mc; // simplest isosurface-extraction algorithm
    //std::vector<ocl::Triangle> tris; // do we need to store all tris here?? no!
    ocl::Octree* tree; // this is the stock model
    ocl::GLData* g; // this is the graphics object drawn on the screen
};

#endif
