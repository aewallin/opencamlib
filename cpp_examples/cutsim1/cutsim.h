#ifndef CUTSIM_H
#define CUTSIM

#include <string>
#include <iostream>
#include <cmath>
#include <vector>

#include <opencamlib/point.h>
#include <opencamlib/triangle.h>
#include <opencamlib/numeric.h>
#include <opencamlib/octree.h>
#include <opencamlib/volume.h>
#include <opencamlib/marching_cubes.h>

class Cutsim  {
public:
    //std::cout << ocl::revision() << "\n";
    //std::cout << " Experimental C++ cutting simulation.\n";
    // Octree(root_scale, max_depth, cp)
    Cutsim () {
        ocl::Point octree_center(0,0,0);
        unsigned int max_depth = 7;
        tree = new ocl::Octree(10.0, max_depth, octree_center );
        std::cout << " tree before init: " << tree->str() << "\n";
        tree->init(3u);
        std::cout << " tree after init: " << tree->str() << "\n";
        ocl::PlaneVolume px_plus(true, 0u, -7);
        ocl::PlaneVolume px_minus(false, 0u, 7);
        ocl::PlaneVolume py_plus(true, 1u, -7);
        ocl::PlaneVolume py_minus(false, 1u, 7);
        ocl::PlaneVolume pz_plus(true, 2u, -7);
        ocl::PlaneVolume pz_minus(false, 2u, 7);
        
        tree->diff_negative_root( &px_plus  );
        tree->diff_negative_root( &px_minus );
        tree->diff_negative_root( &py_plus  );
        tree->diff_negative_root( &py_minus );
        tree->diff_negative_root( &pz_plus  );
        tree->diff_negative_root( &pz_minus );
        
        mc = new ocl::MarchingCubes();
    }
    
    void surf() {
        tris = mc->mc_tree( tree );
        std::cout << " mc() got " << tris.size() << " triangles\n";
    }
    std::vector<ocl::Triangle> getTris() {
        return tris;
    }
private:
    ocl::MarchingCubes* mc;
    std::vector<ocl::Triangle> tris;
    ocl::Octree* tree;
    //return 0;
};

#endif
