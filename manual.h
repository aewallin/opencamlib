// opencamlib documentation in Doxygen format

/*! \mainpage OpenCAMLib user manual
 * 
 * \section intro_sec Introduction
 * OpenCAMLib is a C++ library with Python bindings for creating toolpaths for cnc machines.
 * 
 * This section of the manual describes how to install and use OpenCAMLib.
 * \section install_sec Installation
 *
 * \subsection deb Debian package
 *  
 * \subsection source Building from source
 * How to get the source code and compile it.
 * 
 * Building requires the Boost libraries. Visualizations require VTK.
 * \subsubsection ubuntu On Ubuntu
 * \subsubsection windows On Windows
 * \subsection heekscnc HeeksCNC
 * How to get opencamlib installed and working with HeeksCNC.
 * 
 * \section Drop-Cutter
 * Tutorial and examples using drop-cutter.
 */

/*! \page p2 OpenCAMLib developer manual
 * \section intro_dev Introduction
 * This section of the manual describes the computational geometry and algorithms used in OpenCAMLib.
 * \section Drop-Cutter
 * The drop cutter algorithm drops a cutter, positioned at a predefined (x,y) location
 * down along the z-axis until it makes contact with a triangle.
 * The algorithm is split into three parts, testing the cutter separately against the 
 * three vertices, three edges, and the facet of the triangle.
 * In general the vertex test is trivial, the facet test easy, and the edge test is the most difficult.
 * \subsection dc-gen General
 * The user specifies the (x,y) coordinates of the cutter. These are stored in cl.x and cl.y.
 * The task is to calculate the height of the cutter cl.z
 * \subsection cylcutter Cylindrical endmills (Flat)
 * \subsubsection cylvert Vertex test
 * \subsubsection cylfacet Facet test
 * \subsubsection cyledge Edge test
 * \subsection ballcutter Spherical endmills (Ball-nose)
 * \subsubsection ballvert Vertex test
 * \subsubsection ballfacet Facet test
 * \subsubsection balledge Edge test
 * \subsection bullcutter Toroidal endmills (Bull-nose)
 * \subsubsection bullvert Vertex test
 * \subsubsection bullfacet Facet test
 * \subsubsection bulledge Edge test
 * \subsection optdc Optimizing drop-cutter
 * \subsubsection kdtree Using a kd-tree for finding triangles under the cutter
 * \section Z-slice
 * An algorithm for 'slicing' an STL surface at a defined Z-coordinate. Produces waterline type paths. 
 * These contours can be used as a starting point for area-clearing algorithms in roughing operations.
 * \section cutsim Cutting Simulation
 * Describe cutting simulation here (octree etc).
 * \section References
 * \subsection Books
 * \li O'Rourke, Computational Geometry in C.
 * \li deBerg, Computational Geometry: Algorithms and Applications
 * \subsection Papers  
*/





