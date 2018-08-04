/*  $Id$
*/

/*! \mainpage OpenCAMLib manual
 * 
 * \section intro_sec Introduction
 * OpenCAMLib is a C++ library with Python bindings under LGPL license 
 * for creating toolpaths for cnc machines.
 * 
 *      - hosted at: http://code.google.com/p/opencamlib/
 *      - mailing-list at: http://groups.google.com/group/opencamlib
 *      - svn-commits at: http://groups.google.com/group/opencamlib-svn
 * 
 * This section describes how to obtain, build, and install OpenCAMLib.
 * 
 * The \ref usr-manual section describes how to install and use OpenCAMLib.
 * 
 * The \ref dev-manual section describes the CAM-algorithms in more detail.
 * 
 * \section install_sec Installation
 * As of 2010 March there is no release yet. You have to build from source.
 *  
 * \subsection source Building from source
 * The project is hosted at google-code: http://code.google.com/p/opencamlib/
 * 
 * To checkout the newest OpenCAMLib, run:
 * 
 * <kbd>svn checkout http://opencamlib.googlecode.com/svn/trunk/ opencamlib-read-only
 * </kbd>
 * 
 * Building requires the Boost libraries and a compiler that supports OpenMP. Visualizations require VTK. 
 * It should be as simple as:
 * 
 * <kbd>cmake .
 * 
 * make
 * 
 * make install
 * </kbd>
 * 
 * all done!
 * 
 * \subsubsection ubuntu Debian package On Ubuntu
 * As of SVN r95, OCL builds easily on Ubuntu Hardy through Karmic.
 * OCL will build on Lucid with some tweaks, which are not covered in these
 * brief instructions; proper support for Lucid is planned for the near
 * future.
 * 
 * Use apt-get to install some software that helps build Debian packages:
 *
 * <kbd>shell> apt-get install devscripts build-essential</kbd>
 *
 * Use apt-get to install the packages used by OCL.  The build dependencies
 * are listed in the debian/control file, on the "Build-Depends" line.
 * The following command is appropriate for SVN r95, check the source file
 * for current dependencies for your version:
 *
 * <kbd>shell> apt-get install debhelper libboost-python-dev python-dev python-support </kbd>
 *
 * Next build the OCL source tree into a debian package:
 *
 * <kbd>shell> debuild -us -uc</kbd>
 *
 * This will produce a Debian package (a ".deb" file) of OCL in the parent
 * directory "../", which can be installed with dpkg:
 *
 * <kbd>shell> dpkg -i ../python-opencamlib_*.deb</kbd>
 *
 *
 * \subsubsection windows Building On Windows
 * Text here on how to build on windows.
 * 
 * \subsection OCL with CAD-applications
 * How to get opencamlib installed and working with HeeksCNC.
 * 
 * How to use OCL with Blender, FreeCAD, etc.
 */
 
  
 
/*! \page usr-manual OpenCAMLib user manual
 * 
 * Here text on how to use the library.
 * 
 * \section Drop-Cutter
 * Tutorial and examples using drop-cutter.
 */










/*! \page dev-manual OpenCAMLib developer manual
 * 
 * \section intro_dev Introduction
 * This section of the manual describes the computational geometry and algorithms used in OpenCAMLib.
 * Three-dimensional points and vectors are denoted with <b>bold</b>.
 * 
 * \section Drop-Cutter
 * The drop cutter algorithm drops a cutter, positioned at a predefined (x,y) location
 * down along the z-axis until it makes contact with a triangle.
 * The algorithm is split into three parts, testing the cutter separately against the 
 * three vertices ( vertexDrop() ), three edges ( edgeDrop() ), and the facet ( facetDrop() )of the triangle.
 * In general the vertex test is trivial, the facet test easy, and the edge test is the most difficult.
 * 
 * \subsection dc-gen General
 * A cutter-location <b>cl</b> point is a point where the cutter tip can be located when in contact
 * with the surface, but without interfering with the STL-surface.
 * A cutter-contact (<b>cc</b>) point is the point where the cutter makes contact with the STL-surface.
 * The user specifies the (cl.x, cl.y) coordinates of the cutter, and an initial value cl.z for the cl z-coordinate. 
 * The task for the vertex, facet, and edge tests is to update the height of the cutter cl.z. When each vertex, edge,
 * and facet of each triangle under the cutter positioned at (cl.x, cl.y) has been tested, the resulting cl.z is the 
 * correct and final (maximum) height of the cutter we want. If no test finds a contact between the cutter and a triangle, then 
 * cl.z and <b>cc</b> are left unmodified.
 * 
 * The following milling cutters are supported by opencamlib. They all inherit from the base-class MillingCutter. 
 * The CompoundCutter class allows specifying any shape similar to a general APT-type cutter.
 * 
 * All cutters share common data-fields:
 *      - diameter
 *      - radius
 *      - length
 * 
 * Cutter shapes:
 *      - Cylindrical: CylCutter
 *        - radius = diameter/2
 *      - Spherical:   BallCutter
 *        - radius = diameter/2
 *      - Toroidal:    BullCutter
 *        - radius1 = the radius of the cylindrical middle part of the cutter. also called the torus radius
 *        - radius2 = the tube-radius of the torus
 *        - radius2 <= radius1 must be true (undefined behavior may occur if radius2>radius1)
 *      - Conical:     ConeCutter
 *        - diameter = maximum diameter of the cone 
 *        - angle = half-angle of the cone, in radians. For a 90-degree cutter, set angle = pi/4
 *      - Compound: combinations of the above (restricted to an overall convex cutter shape). For example:
 *        - ConeConeCutter
 *        - BallConeCutter
 *        - BullConeCutter
 * 
 * \subsection verttest Vertex test
 * Milling cutters are symmetrical around their axis of rotation (the z-axis in 3-axis milling), so the vertex
 * test only depends on the distance between the cutter axis and the test-vertex.
 * Given <b>cl</b> and a vertex <b>p</b> to test, calculate the distance d in the xy-plane from <b>cl</b> to <b>p</b>: 
 * \f[
 * d = |cl - p| = \sqrt{(cl.x-p.x)^2+(cl.y-p.y)^2}
 * \f]
 * If \f$d > radius\f$ the cutter will not contact the vertex and we return without modifying cl.z.
 * If \f$d <= radius\f$ the cutter will make contact with the vertex.
 * 
 *      - Cylindrical cutters are flat we can set \f$cl.z = p.z\f$ 
 *      - For a Spherical cutter (see FigureX) \f$h = r - \sqrt{ r^2 - d^2 }\f$ and we set \f$cl.z = p.z - h\f$
 *      - For Toroidal cutters:
 *          - If \f$d < r_1\f$ we are in the cylindrical part of the cutter and set \f$cl.z = p.z\f$
 *          - If \f$r_1 < d < r\f$ we are in the toroidal part and \f$ h = r_2 - \sqrt{ r_2^2 - (d-r_1)^2 }\f$ and then \f$cl.z = p.z - h\f$
 *      - For Conical cutters:
 *          - \f$h = d/tan(angle)\f$ and set \f$cl.z = p.z - h\f$
 * In all cases <b>cc</b> = <b>p</b>. (TODO: insert image of vertex test here)
 * 
 * 
 * 
 * 
 * \subsection facettest Facet test
 * Call the three vertices of the triangle <b>p1</b>, <b>p2</b>, and <b>p3</b>.
 * The facet test is really a test against the whole plane which contains the triangle facet.
 * First calculate the normal <b>n</b> of the facet, and normalize its length to one. The equation for the
 * plane is given by 
 * \f[
 * a*x + b*y + c*z + d = 0
 * \f]
 * where the coefficients \f$(a, b, c) = \bar{n}\f$. The constant d can be calculated by inserting one vertex 
 * into the formula of the plane: \f$d = -\bar{n}\cdot \bar{p_1}\f$.
 * 
 * Also calculate and normalize the xy-projection of the normal \f$\bar{n_{xy}}\f$. 
 * Note that vertical facets with n.z = 0 are a special case which are ignored in the drop-cutter algorithm.
 * 
 * \subsubsection cylfacettest Cylindrical cutter facet test
 * The contact point with the plane will lie on the periphery of the cutter, a
 * distance \f$radius\f$ from \f$(cl.x, cl.y)\f$ in the direction \f$-\bar{n_{xy}}\f$, so the xy-coordinates of 
 * the cc-point are given by:
 * \f[
 * (cc.x, cc.y) = (cl.x, cl.y) - r*\bar{n_{xy}}
 * \f]
 * This cc-point lies on the plane, but not necessarily in the facet of the triangle. 
 * If the cc-point is contained within the triangle (in the xy-plane), we can calculate its z-coordinate by
 * inserting into the equation of the plane:
 * \f[
 * cc.z = (1/c)*(-d-a*cc.x-b*cc.y)
 * \f] 
 * Since the cylindrical endmill is flat, we can also set \f$cl.z = cc.z\f$. 
 * Note division by \f$c\f$ which requires \f$c = n.z \neq 0\f$
 * 
 * \subsubsection sphfacettest Spherical cutter facet test
 * The center of the sphere is located a distance \f$radius\f$  from \f$(cl.x, cl.y)\f$ in the direction of \f$-\bar{n}\f$. 
 * We can find out the xy-compontents of the cc point with:
 * \f[
 * \bar{cc} = \bar{cl} - r*\bar{n}
 * \f]
 * If this point lies within the facet of the triangle we locate the point on the plane by setting cc.z on the plane:
 * \f[
 * cc.z = (1/c)*(-d-a*cc.x-b*cc.y)
 * \f] 
 * The tool-tip z-coordinate is now given by \f$cl.z = cc.z + r*(n\cdot\hat{z})  - r \f$
 * 
 * 
 * \subsubsection torfacettest Toroidal cutter facet test
 * Define a vector which points from the cc-point to the center of the toroid. This is a distance \f$radius2\f$
 * along the surface normal \bar{n} and a distance \f$radius1\f$ along the XY-normal \bar{n_{xy}}:
 * \f[
 * \bar{R} = r_2*\bar{n} + r_1* \bar{n_{xy}}
 * \f]  
 * Now the xy-coordinates of the cc-point are given by
 * \f[
 * \bar{cc} = \bar{cl} - \bar{R}
 * \f] 
 * now check if this cc-point lies in the facet of the plane. If so, the z-coordinate of the cc-point is calculated:
 * \f[
 * cc.z = (1/c)*(-d-a*cc.x-b*cc.y)
 * \f]  
 * And the tool-tip will be located at \f$cl.z = cc.z + r_2*n.z - r_2\f$
 * 
 * \subsubsection conefacettest Conical cutter facet test
 * Describe ConeCutter facet test here.
 * 
 * \subsection edgetest Edge test
 * Each test loops through all three edges of the triangle. The edge is defined by its startpoint
 * <b>p1</b> and its endpoint <b>p2</b>.
 * 
 * It is easier to do the tests in a translated and rotated coordinate system where the
 * cl-point is at (0,0) and the edge is rotated to be parallel with the X-axis. We call the 
 * new endpoints of the edge <b>p1u</b> and <b>p2u</b>. These are found by noting that
 * \f[
 * p1u_y = d
 * p2u_y = d
 * \f]  
 * where d is the distance, in the XY-plane, from cl to a line through p1 and p2, 
 * i.e. \f$d = cl.xyDistanceToLine(p1, p2) \f$. We also define, in the XY-plane, 
 * the closest point to cl on the line as: \f$ sc = cl.xyClosestPoint( p1, p2 ) \f$.
 * 
 * Now the x-coordinates of <b>p1u</b> and <b>p2u</b> are given by:
 * \f[
 * p1u_x = (p1-sc).dot(v)
 * p2u_x = (p2-sc).dot(v)
 * \f]
 * where v is a normalized vector in the direction of the edge, or v=(p2-p1).xyNormalize()
 * 
 * The tests find a contact between the cutter and an infinite line which contains the
 * edge we are testing against. Once the contact cc-point is found we check whether this cc-point is
 * actually in the edge.
 * 
 * \subsubsection cyledge Cylindrical cutter edge test
 * In the XY-plane we want to find the intersection between an infinite line and a circle.
 * This is described e.g. here: see http://mathworld.wolfram.com/Circle-LineIntersection.html
 * 
 * \subsubsection balledge Spherical cutter edge test
 * \subsubsection bulledge Toroidal cutter edge test
 * \subsubsection coneedge Conical cutter edge test
 * 
 * \subsection optdc Optimizing drop-cutter
 * \subsubsection kdtree Using a kd-tree for finding triangles under the cutter
 * 
 * 
 * 
 * \section Z-slice
 * An algorithm for 'slicing' an STL surface at a defined Z-coordinate. Produces waterline type paths. 
 * These contours can be used as a starting point for area-clearing algorithms in roughing operations.
 * 
 * 
 * \section cutsim Cutting Simulation
 * Describe cutting simulation here (octree etc).
 * 
 * 
 * \section References
 * \subsection Books
 * \li O'Rourke, Computational Geometry in C.
 * \li deBerg, Computational Geometry: Algorithms and Applications
 * \li Ghali, Introduction to Geometric Computing
 * 
 * \subsection Papers  
 * \li Chuang2002  http://dx.doi.org/10.1007/PL00003965 "A Reverse Engineering Approach to Generating Interference-Free Tool 
 * Paths in Three-Axis Machining from Scanned Data of Physical Models"
 *      - this paper contains the basics of drop-cutter
 * \li Hwang1998 http://dx.doi.org/10.1016/S0010-4485(98)00021-9 
 *      - DropCutter for flat, filleted and ball-nose cutters. Edge test with iterative solution. 
 * \li Yau2004 http://dx.doi.org/10.1080/00207540410001671651 
 *      - DropCutter for a general APT-style cutter. This includes cutters with pointed ends and tapered cutters. 
 *      - kd-tree for triangle search
 * \li Chuang2005 http://dx.doi.org/10.1016/j.cad.2004.10.005 
 *      - presents a Push-cutter idea for calculating z-level (waterline) paths. 
 * 
*/





