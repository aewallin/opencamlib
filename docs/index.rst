.. opencamlib documentation master file, created by
   sphinx-quickstart on Sat Aug  4 21:55:45 2018.
   You can adapt this file completely to your liking, but it should at least
   contain the root `toctree` directive.

OpenCAMLib
==========

Introduction
============

OpenCAMLib (ocl) is a c++ library with python bindings for creating 3D toolpaths for cnc-machines such as mills and lathes. From August 2018 OpenCAMLib is released under LGPL license.

repository https://github.com/aewallin/opencamlib
mailing-list http://groups.google.com/group/opencamlib
IRC-channel #cam on irc.freenode.net
coding standard (?) http://www.possibility.com/Cpp/CppCodingStandard.html
BUILDING and INSTALLING
to clone, build and install install the ocl.so library and camvtk.py run the following:

.. code-block:: console

   git clone git://github.com/aewallin/opencamlib.git
   cd opencamlib
   mkdir build
   cd build
   cmake ../src
   make        (try make -j4 for a faster build if you have a multi-core machine)
   sudo make install

if you also want to build the documentation, then run:

.. code-block:: console

   make doc
   make doc-pdf

this should create the ocl manual named "ocl-manual.pdf" in the build/doc directory

using cmake and cpack it is possible to build a binary .deb package with:

.. code-block:: console

   make package

For uploading debian source-packages to the PPA there is also a custom target:

.. code-block:: console

   make spackage

The build directory opencamlib/build can be wiped clean ("rm -rf *") and cmake run again if/when you want a clean build.

.. toctree::
   :maxdepth: 1
   :caption: Contents:

   api


Indices and tables
==================

* :ref:`genindex`
* :ref:`modindex`
* :ref:`search`

