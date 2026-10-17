/*  $Id$
 * 
 *  Copyright (c) 2010-2011 Anders Wallin (anders.e.e.wallin "at" gmail.com).
 *  
 *  This file is part of OpenCAMlib 
 *  (see https://github.com/aewallin/opencamlib).
 *  
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 2.1 of the License, or
 *  (at your option) any later version.
 *  
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *  
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef KDTREE_H
#define KDTREE_H

#include <array>
#include <cassert>
#include <memory>
#include <vector>
#include <iostream>
#include <list>


#include "kdnode.hpp"
#include "bbox.hpp"
#include "millingcutter.hpp"
#include "clpoint.hpp"
#include "numeric.hpp"

namespace ocl
{
    
class Point;
class CLPoint;
class Triangle;
class MillingCutter;

/// \brief KDTree spread, a measure of how spread-out a list of triangles are.
///
/// simple struct-like class for storing the "spread" or maximum 
/// extent of a list of triangles. Used by the kd-tree algorithm.
class Spread {
    public:
        /// constructor
        Spread(int dim, double v, double s) {
            d = dim;
            val = v;
            start = s;
        };
        /// dimension
        int d;
        /// spread-value
        double val;
        /// minimum or start value
        double start;
        /// comparison of Spread objects. Used for finding the largest spread
        /// along which the next partition/cut is made.
        static bool spread_compare(Spread *x, Spread *y) {
            if (x->val > y->val)
                return true;
            else
                return false;
        };
};

/// a kd-tree for storing triangles and fast searching for triangles
/// that overlap the cutter
template <class BBObj>
class KDTree {
    public:
        KDTree() : bucketSize(1), root(nullptr) {}
        virtual ~KDTree() {
            // std::cout << " ~KDTree()" << std::endl;
	    delete root;
        }
        /// set the bucket-size 
        void setBucketSize(int b){
            //std::cout << "KDTree::setBucketSize = " << b << "\n"; 
            bucketSize = b;
        }
        /// set the search dimension to the XY-plane
        void setXYDimensions(){
            //std::cout << "KDTree::setXYDimensions()\n"; 
            dimensions.clear();
            dimensions.push_back(0); // x
            dimensions.push_back(1); // x
            dimensions.push_back(2); // y
            dimensions.push_back(3); // y
        } // for drop-cutter search in XY plane
        /// set search-plane to YZ
        void setYZDimensions(){ // for X-fibers
            //std::cout << "KDTree::setYZDimensions()\n"; 
            dimensions.clear();
            dimensions.push_back(2); // y
            dimensions.push_back(3); // y
            dimensions.push_back(4); // z
            dimensions.push_back(5); // z
        } // for X-fibers
        /// set search plane to XZ
        void setXZDimensions(){ // for Y-fibers
            //std::cout << "KDTree::setXZDimensions()\n";
            dimensions.clear();
            dimensions.push_back(0); // x
            dimensions.push_back(1); // x
            dimensions.push_back(4); // z
            dimensions.push_back(5); // z
        } // for Y-fibers
        /// build the kd-tree based on a list of input objects
        void build(const std::list<BBObj>& list) {
            assert(!dimensions.empty());
            KDNode<BBObj>* replacement = build_node(&list, 0, nullptr);
            delete root;
            root = replacement;
        }
        /// search for overlap with input Bbox bb, return found objects
        std::list<BBObj>* search( const Bbox& bb ){
            assert( !dimensions.empty() );
            std::list<BBObj>* tris = new std::list<BBObj>();
            this->search_node( tris, bb, root );
            return tris;
        }
        /// search for overlap with a MillingCutter c positioned at cl, return found objects
        std::list<BBObj>* search_cutter_overlap(const MillingCutter* c, CLPoint* cl ){
            double r = c->getRadius();
            // build a bounding-box at the current CL
            Bbox bb( cl->x-r, cl->x+r, cl->y-r, cl->y+r, cl->z, cl->z+c->getLength() );    
            return this->search( bb );
        }
        /// Visit candidates in search order without copying them or allocating a result list.
        /// References belong to the tree and remain valid until it is rebuilt or destroyed.
        /// Concurrent visits are safe if callbacks do not modify the tree or shared state.
        template<class Visitor>
        void visit(const Bbox& bb, Visitor&& visitor) const {
            assert(!dimensions.empty());
            visit_node(bb, root, visitor);
        }
        template<class Visitor>
        void visit_cutter_overlap(const MillingCutter* c, const CLPoint* cl,
                                  Visitor&& visitor) const {
            const double r = c->getRadius();
            const Bbox bb(cl->x-r, cl->x+r, cl->y-r, cl->y+r, cl->z, cl->z+c->getLength());
            visit(bb, visitor);
        }
        /// string repr
        std::string str() const;
        
    protected:
        /// Take one owned snapshot; partition by relinking nodes, not copying triangles.
        KDNode<BBObj>* build_node(const std::list<BBObj>* tris, int dep, KDNode<BBObj>* par) {
            std::list<BBObj> owned(*tris);
            return partition_node(owned, dep, par);
        }

        KDNode<BBObj>* partition_node(std::list<BBObj>& tris, int dep, KDNode<BBObj>* par) {
            if (tris.empty())
                return nullptr;
            const Spread spread = spread_value(tris);
            const double cutvalue = spread.start + spread.val/2;
            if (tris.size() <= bucketSize || isZero_tol(spread.val)) {
                const std::list<BBObj> empty;
                std::unique_ptr<KDNode<BBObj>> bucket(
                    new KDNode<BBObj>(spread.d, cutvalue, par, nullptr, nullptr, &empty, dep));
                bucket->tris->splice(bucket->tris->end(), tris);
                return bucket.release();
            }
            std::list<BBObj> lo, hi;
            for (auto it = tris.begin(); it != tris.end();) {
                auto current = it++;
                auto& partition = current->bb[spread.d] > cutvalue ? hi : lo;
                partition.splice(partition.end(), tris, current);
            }
            std::unique_ptr<KDNode<BBObj>> node(
                new KDNode<BBObj>(spread.d, cutvalue, par, nullptr, nullptr, nullptr, dep));
            node->hi = partition_node(hi, dep+1, node.get());
            node->lo = partition_node(lo, dep+1, node.get());
            return node.release();
        }

        Spread* calc_spread(const std::list<BBObj>* tris) {
            return new Spread(spread_value(*tris));
        }

        Spread spread_value(const std::list<BBObj>& tris) const {
            assert(!tris.empty() && !dimensions.empty());
            std::array<double, 6> minval{}, maxval{};
            for (int dim : dimensions)
                minval[dim] = maxval[dim] = tris.front().bb[dim];
            for (const BBObj& object : tris) {
                for (int dim : dimensions) {
                    const double value = object.bb[dim];
                    if (value > maxval[dim]) maxval[dim] = value;
                    if (value < minval[dim]) minval[dim] = value;
                }
            }
            int widest = dimensions.front();
            double width = 0;
            for (int dim : dimensions) {
                const double value = maxval[dim] - minval[dim];
                if (value > width) {
                    width = value;
                    widest = dim;
                }
            }
            return Spread(widest, maxval[widest]-minval[widest], minval[widest]);
        }

        /// Keep the owning search API for callers that need independent copies.
        void search_node(std::list<BBObj>* tris, const Bbox& bb, KDNode<BBObj>* node) {
            auto append = [tris](const BBObj& object) { tris->push_back(object); };
            visit_node(bb, node, append);
        }

        template<class Visitor>
        void visit_node(const Bbox& bb, const KDNode<BBObj>* node, Visitor& visitor) const {
            if (!node)
                return;
            if (node->isLeaf) {
                for (const BBObj& object : *node->tris)
                    visitor(object);
            } else if ((node->dim % 2) == 0 && node->cutval > bb[node->dim+1]) {
                visit_node(bb, node->lo, visitor);
            } else if ((node->dim % 2) != 0 && node->cutval < bb[node->dim-1]) {
                visit_node(bb, node->hi, visitor);
            } else {
                // Preserve hi-before-lo order: equal-height contacts depend on it.
                visit_node(bb, node->hi, visitor);
                visit_node(bb, node->lo, visitor);
            }
        }
    // DATA
        /// bucket size of tree
        unsigned int bucketSize;
        /// pointer to root KDNode
        KDNode<BBObj>* root;
        /// the dimensions in this kd-tree
        std::vector<int> dimensions;
};

} // end ocl namespace
#endif
// end file kdtree.hpp
