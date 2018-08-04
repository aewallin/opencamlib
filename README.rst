.. image:: https://img.shields.io/badge/License-LGPL%20v2.1-blue.svg
    :target: https://www.gnu.org/licenses/old-licenses/lgpl-2.1.en.html
    
.. image:: https://travis-ci.org/aewallin/opencamlib.svg?branch=master
    :target: https://travis-ci.org/aewallin/opencamlib

.. image:: https://readthedocs.org/projects/opencamlib/badge/?version=latest
    :target: https://opencamlib.readthedocs.io/en/latest/?badge=latest
    :alt: Documentation Status

OpenCAMLib README
=================



INTRODUCTION
---------------

OpenCAMLib (ocl) is a c++ library with python bindings for creating 3D toolpaths for cnc-machines
such as mills and lathes. From August 2018 OpenCAMLib is released under LGPL license.

- repository https://github.com/aewallin/opencamlib
- mailing-list http://groups.google.com/group/opencamlib
- IRC-channel #cam on irc.freenode.net
- coding standard (?) http://www.possibility.com/Cpp/CppCodingStandard.html


BUILDING and INSTALLING 
-----------------------

to clone, build and install install the ocl.so library and camvtk.py run the following::

 $ git clone git://github.com/aewallin/opencamlib.git
 $ cd opencamlib
 $ mkdir build
 $ cd build
 $ cmake ../src
 $ make        (try make -j4 for a faster build if you have a multi-core machine)
 $ sudo make install

if you also want to build the documentation, then run::

 $ make doc
 $ make doc-pdf
 
this should create the ocl manual named "ocl-manual.pdf" in the build/doc directory

using cmake and cpack it is possible to build a binary .deb package with::

 $ make package
 
For uploading debian source-packages to the PPA there is also a custom target::

 $ make spackage

The build directory opencamlib/build can be wiped clean ("rm -rf *") and cmake run 
again if/when you want a clean build.

ORGANIZATION OF FILES
---------------------

(generate this with 'tree -dL 2')::

 ├── cpp_examples                c++ example use of opencamlib
 │   ├── point                   minimal example of ocl::Point
 ├── debian                      files for building a debian package
 ├── doc                         documentation (not much here yet!)
 ├── lib                         useful python helper libraries
 ├── scripts                     python scripts that test or demonstrate use of ocl
 │   ├── issues
 │   ├── ocode                   old linear octree code
 │   ├── offset-ellipse          relates to BullCutter drop- and push-cutter
 │   ├── old
 │   └── voronoi                 vd scripts/tests
 ├── src
 │   ├── algo                    algorithms under development
 │   ├── attic                   old deprecated code
 │   ├── common                  common algorithms and data-structures
 │   ├── cutters                 cutter-classes
 │   ├── dropcutter              drop-cutter algorithms and operations
 │   ├── geo                     primitive geometry classes (point,triangle,stlsurf, etc.)
 │   └── voronoi
 ├── stl                         STL files for testing
 └── Windows                     Visual-studio project for building on windows


Ubuntu 10.04LTS-> install and build
-----------------------------------

(these notes are about 8 years old now and should be updated for relevant OSs)

- install 64-bit Ubuntu 10.04LTS
- run update manager and install all suggested updates. reboot.
- using synaptic or apt-get, install the following packages(and their dependencies)
 - build-essential
 - cmake
 - subversion
 - doxygen
 - texlive-latex-base
 - libboost-all-dev
- checkout ocl from svn
 - instructions: http://code.google.com/p/opencamlib/source/checkout
- in the /src directory, first run "cmake ." then "make" and then "sudo make install"
 - this should build and install ocl correctly.


