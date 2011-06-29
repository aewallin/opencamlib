/*  $Id:  $
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

#ifndef CUTSIM_H
#define CUTSIM_H

#include <QObject>

#include <string>
#include <iostream>
#include <cmath>
#include <vector>
#include <ctime>

//#include <functional>
#include <boost/bind.hpp>
#include <boost/timer.hpp>


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
        tree->init(3u);
        tree->debug=false;
        std::cout << " tree after init: " << tree->str() << "\n";
        
        /*
        ocl::SphereOCTVolume stock_sphere;
        stock_sphere.radius = 7;
        stock_sphere.center = ocl::Point(0,0,0);
        stock_sphere.calcBB();
        stock_sphere.invert = true;
        
        tree->diff_negative( &stock_sphere );
        */
        ocl::CubeVolume cube;
        cube.side = 11.0;
        cube.invert = true;
        cube.calcBB();
        tree->diff_negative( &cube);
        
        
        std::cout << " tree after sphere-cut: " << tree->str() << "\n";
        
        mc = new ocl::MarchingCubes();
        tree->setIsoSurf(mc);

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
        tree->updateGL();
        std::cout << " updateGL() \n";
    }

public slots:
    void cut() { // demo slot of doing a cutting operation on the tree with a volume.
        std::cout << " cut! called \n";
        ocl::SphereOCTVolume s;
        s.radius = 2;
        s.center = ocl::Point(4,4,4);
        s.calcBB();
        //std::cout << " before diff: " << tree->str() << "\n";
        std::clock_t start, stop;
        boost::timer tim;
        
        tim.restart();
        start = std::clock();
        tree->diff_negative( &s );
        stop = std::clock();
        
        std::cout << " diff(): " << ( ( stop - start ) / (double)CLOCKS_PER_SEC ) <<'\n';
        std::cout << " diff(): " << tim.elapsed() <<'\n';
        
        tim.restart();
        start = std::clock();
        //g->updateVBO();
        updateGL();
        stop = std::clock();
        
        std::cout << " updateGL(): " << ( ( stop - start ) / (double)CLOCKS_PER_SEC ) <<'\n';
        std::cout << " updateGL(): " << tim.elapsed() <<'\n';
        //std::cout << " AFTER diff: " << tree->str() << "\n";

        tim.restart();
        start = std::clock();
        g->updateVBO();
        stop = std::clock();
        std::cout << " updateVBO(): " << ( ( stop - start ) / (double)CLOCKS_PER_SEC ) <<'\n';
        std::cout << " updateVBO(): " << tim.elapsed() <<'\n';
    }
    
private:
    ocl::MarchingCubes* mc; // simplest isosurface-extraction algorithm
    //std::vector<ocl::Triangle> tris; // do we need to store all tris here?? no!
    ocl::Octree* tree; // this is the stock model
    ocl::GLData* g; // this is the graphics object drawn on the screen
};

#endif
