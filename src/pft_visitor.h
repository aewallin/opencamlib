
#ifndef PFT_VISITOR_H
#define PFT_VISITOR_H

#include <vector>

#include <boost/graph/adjacency_list.hpp> // graph class
#include <boost/graph/breadth_first_search.hpp>
#include <boost/graph/graphviz.hpp>
#include <boost/graph/planar_face_traversal.hpp>

#include "weave_typedef.h"
#include "weave.h"
namespace ocl
{
    
    
struct vertex_output_visitor : public boost::planar_face_traversal_visitor
{
    vertex_output_visitor(Weave* w, WeaveGraph& g) : g_(g) , w_(w) {}
    
    void begin_face() { 
        current_loop.clear();
    } 
    
    void next_vertex(VertexDescriptor v) { 
        if ( boost::get( boost::vertex_type, g_, v) == CL ) {
            current_loop.push_back(v);
        }
    }
    
    void end_face() { 
        if (!current_loop.empty())
            w_->loops.push_back( current_loop );
    } 
    
    WeaveGraph& g_;
    Weave* w_;
    std::vector<VertexDescriptor> current_loop;
};

} // end namespace
#endif
