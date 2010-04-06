/*  Copyright 2010 Anders Wallin (anders.e.e.wallin "at" gmail.com)
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

#ifndef OCTREE_H
#define OCTREE_H

#include <iostream>
#include <list>
#include <vector>

class Point;
class Triangle;
class MillingCutter;

/// base-class for defining volumes to build octrees
class OCTVolume {
    public:
        /// return true if Point p is inside volume
        virtual bool isInside(Point& p) const = 0;
};

/* required wrapper class for virtual functions in boost-python */
/// \brief a wrapper around OCTVolume required for boost-python
class OCTVolumeWrap : public OCTVolume, public bp::wrapper<OCTVolume>
{
    public:
    bool isInside(Point &p) const
    {
        return this->get_override("isInside")(p);
    }
};

class SphereOCTVolume: public OCTVolume {
    public:
        SphereOCTVolume();
        Point center;
        double radius;
        bool isInside(Point& p) const;
};

class CubeOCTVolume: public OCTVolume {
    public:
        CubeOCTVolume();
        Point center;
        double side;
        bool isInside(Point& p) const;
};


/// type of octree node
enum OCType { WHITE, GREY, BLACK };

/// \brief A node in a octree. http://en.wikipedia.org/wiki/Octree
///
class OCTNode {
    public:
        /// empty default constructor
        OCTNode();
        
        /// Create a node 
        OCTNode(int level, Point &center, OCType type, OCTNode *parent);
                                
        /// string repr
        std::string str();
        /// string repr
        friend std::ostream &operator<<(std::ostream &stream, const OCTNode node);
        
        
        /// level of node in tree 
        int level;
        /// center Point of node
        Point center;
        /// scale
        double scale;
        /// type of node
        OCType type;
        /// pointer to parent-node
        OCTNode *parent;
        /// pointers to the eight Child-nodes
        std::vector<OCTNode*> child;
        
        /// return a node-point    
        Point nodePoint(int id);
        
        /// return direction to node-point
        Point nodeDir(int id);
        
        /// return center-point of child
        Point childCenter(int id);
        
        /// side of cube at level=0    
        static double max_scale;
        /// maximum depth of tree
        static int max_depth;
        /// minimum depth of tree
        static int min_depth;
        
        /* static functions to build and manipulate OCTrees */
        /// build a kd-tree from a list of triangles. return root of tree.
        static OCTNode* build_octree(OCTVolume* vol, int level, Point& center, OCTNode* parent);
        
        /// append all nodes under root to list nodes
        static void getNodes( std::list<OCTNode> *nodes, OCTNode *node);
        
        static void prune(OCTNode* root);
        
        static int prune_all(OCTNode* root);
        
        
        /// balance root2 so that it matches  root1
        static void balance(OCTNode* root1, OCTNode* root2);
        
        static void diff(OCTNode* root1, OCTNode* root2);
        
        
        /// return the max scale
        double get_max_scale();
};


/// class for testing and debugging octree through python
class OCTest {
    public:
        OCTest();
        OCTNode* root;
        OCTVolume* volume;
        void build_octree();
        boost::python::list get_all_nodes();
        boost::python::list get_white_nodes();
        boost::python::list get_black_nodes();
        
        double get_max_depth();
        void set_max_depth(int d);
        void prune();
        int prune_all();
        
        /// set volume object for building octree
        void setVol(OCTVolume& volume);
        void balance(OCTest& other);
        void diff(OCTest& other);
        
};

#endif
