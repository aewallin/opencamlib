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
 *
 * As of SVN r95, OCL builds easily on Ubuntu Hardy through Karmic.
 * OCL will build on Lucid with some tweaks, which are not covered in these
 * brief instructions; proper support for Lucid is planned for the near
 * future.
 * 
 * Use apt-get to install some software that helps build Debian packages:
 *
 * shell> apt-get install devscripts build-essential
 *
 * Use apt-get to install the packages used by OCL.  The build dependencies
 * are listed in the debian/control file, on the "Build-Depends" line.
 * The following command is appropriate for SVN r95, check the source file
 * for current dependencies for your version:
 *
 * shell> apt-get install debhelper libboost-python-dev python-dev python-support
 *
 * Next build the OCL source tree into a debian package:
 *
 * shell> debuild -us -uc
 *
 * This will produce a Debian package (a ".deb" file) of OCL in the parent
 * directory "../", which can be installed with dpkg:
 *
 * shell> dpkg -i ../python-opencamlib_*.deb
 *
 *
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
 * Three-dimensional points and vectors are denoted with <b>bold</b>.
 * \section Drop-Cutter
 * The drop cutter algorithm drops a cutter, positioned at a predefined (x,y) location
 * down along the z-axis until it makes contact with a triangle.
 * The algorithm is split into three parts, testing the cutter separately against the 
 * three vertices, three edges, and the facet of the triangle.
 * In general the vertex test is trivial, the facet test easy, and the edge test is the most difficult.
 * \subsection dc-gen General
 * A cutter-location (cl) point is a point where the cutter tip can be located when in contact
 * with the surface, but without interfering with the STL-surface.
 * A cutter-contact (cc) point is the point where the cutter makes contact with the STL-surface.
 * The user specifies the (cl.x, cl.y) coordinates of the cutter, and an initial value cl.z for the cl z-coordinate. 
 * The task for the vertex, facet, and edge tests is to update the height of the cutter cl.z. When each vertex, edge,
 * and facet of each triangle under the cutter positioned at (cl.x, cl.y) has been tested, the resulting cl.z is the 
 * correct and final height of the cutter we want. 
 * \subsection cylcutter Cylindrical endmills (Flat)
 * \subsubsection cylvert Vertex test
 * Given <b>cl</b> and a vertex <b>p</b> to test, calculate the distance d in the xy-plane from <b>cl</b> to <b>p</b>: 
 * \f[
 * d = |cl - p|= \sqrt{(cl.x-p.x)^2+(cl.y-p.y)^2}
 * \f]
 * If \f$d <= diameter/2\f$ the cutter will make contact with the vertex, and we can set \f$cl.z = p.z\f$ .
 * If \f$d > diameter/2\f$ the cutter will not contact the vertex and we return without modifying cl.z.
 * \subsubsection cylfacet Facet test
 * Call the three vertices of the triangle <b>p1</b>, <b>p2</b>, and <b>p3</b>.
 * The facet test is really a test against the whole plane which contains the triangle facet.
 * First calculate the normal <b>n</b> of the facet, and normalize its length to one. The equation for the
 * plane is given by 
 * \f[
 * a*x + b*y + c*z + d = 0
 * \f]
 * where the coefficients \f$(a, b, c) = n\f$. The constant d can be calculated by inserting one vertex 
 * into the formula of the plane: \f$d = -n\cdot p1\f$ .
 * The contact point with the plane will lie on the periphery of the cutter, a
 * distance \f$diameter/2\f$ from \f$(cl.x, cl.y)\f$ in the direction \f$-n\f$, so the xy-coordinates of the cc-point are given by:
 * \f[
 * (cc.x, cc.y) = (cl.x, cl.y) - (diameter/2)*n
 * \f]
 * This cc-point lies on the plane, but not necessarily in the facet of the triangle. 
 * If the cc-point is contained within the triangle (in the xy-plane), we can calculate its z-coordinate by
 * inserting into the equation of the plane:
 * \f[
 * cc.z = (1/c)*(-d-a*cc.x-b*cc.y)
 * \f] 
 * Since the cylindrical endmill is flat, we can also set \f$cl.z = cc.z\f$. Done.
 * \subsubsection cyledge Edge test
 * Explain the edge-test here...
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





