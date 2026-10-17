##########
OpenCAMLib
##########

.. image:: https://img.shields.io/badge/License-LGPL%20v2.1-blue.svg
    :target: https://www.gnu.org/licenses/old-licenses/lgpl-2.1.en.html

.. image:: https://github.com/aewallin/opencamlib/actions/workflows/test.yml/badge.svg
    :target: https://github.com/aewallin/opencamlib/actions/workflows/test.yml

.. image:: https://readthedocs.org/projects/opencamlib/badge/?version=latest
    :target: https://opencamlib.readthedocs.io/en/latest/?badge=latest
    :alt: Documentation Status

************
Introduction
************

OpenCAMLib (ocl) is a library for creating 3D toolpaths for CNC-machines such as mills and lathes.
It is written in C++ and has bindings for Python, Node.js and the browser.
At the moment it supports the following algorithms:

===========
Drop-cutter
===========

The drop cutter algorithm drops a cutter, positioned at a predefined (x,y) location, until it touches the 3D model.

.. image:: https://github.com/aewallin/opencamlib/blob/master/docs/drop-cutter.png?raw=true
  :width: 200
  :alt: Drop Cutter

===========
Push-cutter
===========

The Push-cutter is used to create a Waterline toolpath that follows the shape of the model at a constant z-height in the xy-plane.

.. image:: https://github.com/aewallin/opencamlib/blob/master/docs/push-cutter.png?raw=true
  :width: 200
  :alt: Push Cutter

=======
Cutters
=======

The algorithms listed above can be used with following cutters:

- **CylCutter** (flat end mill / cylindrical)
- **BallCutter** (ball end mill / spherical)
- **BullCutter** (radius end mill / toroidal)
- **ConeCutter** (tapered end mill / conical)
- **CompositeCutter** (combinations of the above / compound)

From August 2018 OpenCAMLib is released under LGPL license.

**********************
Pre-compiled Libraries
**********************

OpenCAMLib provides pre-compiled C++, Node.js and Python libraries for the following platforms and architectures:

+-------------+------------------+
| **Windows** | ia32 / x64       |
+-------------+------------------+
| **macOS**   | x86_64 / arm64   |
+-------------+------------------+
| **Linux**   | x86_64 / aarch64 |
+-------------+------------------+

- The Python library is called ``opencamlib`` and is hosted on PyPi (pypi.org), precompiled libraries are available for Python v3.9 up to v3.14.
- The Node.js + emscripten library is called ``@opencamlib/opencamlib`` and is hosted on npm (npmjs.org), precompiled libraries are available for Node-API v3 and up.
- The C++ library is called ``libocl`` and is hosted on our Github Releases page.

======
Python
======

The Python library (hosted on PyPi) can be installed like this:

..  code-block:: shell

    pip install opencamlib

On some platforms, pip is called pip3, you might have to run:

..  code-block:: shell

    pip3 install opencamlib

Note that pip / pip3 is will install packages for to the system installation of Python, if you want to install a package in a custom Python installation that is not in your $PATH
(for example, the Python which comes with Blender), you can install packages like so:

..  code-block:: shell

    /path/to/your/custom/python -m pip install opencamlib

If you don't know where Python is, but you have access to it's interpreter (FreeCAD and Blender both have a Python console), you can simply enter this command in there to install OpenCAMLib:

..  code-block:: python

    import sys; import subprocess; subprocess.check_call([sys.executable, '-m', 'pip', 'install', 'opencamlib'])


==========
JavaScript
==========

The JavaScript library (hosted on npm) works in Node.js and the browser (by leveraging emscripten / WASM) can be installed like this:

..  code-block:: shell

    npm install --save @opencamlib/opencamlib

Or, using yarn:

..  code-block:: shell

    yarn add @opencamlib/opencamlib

Note that it is not 100% feature complete and lacking some functionality still.

===
C++
===

Pre-compiled C++ libraries are available on the Github Releases page (https://github.com/aewallin/opencamlib/releases).
This project also installs a OpenCAMLibConfig.cmake, which, if your project uses CMake, allows you to use ``find_package(OpenCAMLib REQUIRED)``.

You can see an example of that in use over here: ``examples/cpp/test/CMakeLists.txt``

********************
Building from Source
********************

Having trouble with a pre-compiled library? Please report it to us.
If there are no pre-compiled libraries for your platform or architecture, or want to customize or package opencamlib, this is for you.

The core C++ algorithms use the C++ standard library and project-owned data structures.
OpenMP is optional; configure with ``-D USE_OPENMP=OFF`` to build without it.

We provide a ``install.sh`` script that helps with installation of dependencies and building OpenCAMLib libraries, you might want to take a look at it first.
You can run ``./install.sh --help`` to look at the available options, or inspect it's source code to find out more.

============
Dependencies
============

To compile OpenCAMLib, you need:

- **C++ compiler** (It should at least support C++ 14)
- **Git** (This is used for cloning the repository, and the emscripten SDK)
- **CMake** (At least version 3.18)

Building the Python bindings additionally requires Python development headers.
pybind11 is installed automatically when building the package with pip.

At this time of writing, here are the packages to install:

Ubuntu Dependencies
-------------------

..  code-block:: shell

    sudo apt install -y git cmake curl build-essential python3-dev python3-pip

macOS Dependencies
------------------

..  code-block:: shell

    brew install git cmake curl libomp python

Windows Dependencies
--------------------

Install

- Visual Studio Build Tools (https://visualstudio.microsoft.com/visual-cpp-build-tools/)
- Git (https://git-scm.com/download/win)
- CMake (https://cmake.org/download/)

By downloading the installers from the internet, or by using your package manager.

================
Building for C++
================

The C++ library requires a C++14 compiler, Git, and CMake.
Install OpenMP for parallel computation, or configure with ``-D USE_OPENMP=OFF``.

..  code-block:: shell

    git clone https://github.com/aewallin/opencamlib
    cd opencamlib
    mkdir build
    cd build
    cmake .. -D BUILD_CXX_LIB="ON"
    cmake --build . # try make --parallel for a faster build if you have a multi-core machine
    cmake --install .

The native regression tests build by default. Run ``ctest --output-on-failure``
from the build directory, or set ``-D BUILD_TESTING=OFF`` to omit them.

The half-edge graph in ``src/common/halfedgediagram.hpp`` owns its storage and
adjacency operations. Vertex and edge handles are distinct index types; insertion
preserves existing handles and property references, and deletion makes a slot
available for reuse. Graph enumeration returns snapshots. Code using the low-level
graph template should use ``HEDIGraph<VertexProps, EdgeProps, FaceProps>`` and the
``hedi::VertexDescriptor`` / ``hedi::EdgeDescriptor`` types. Rebuild C++ consumers
when upgrading because the graph and interval layouts have changed.

Read-only adjacency traversal can use ``out_edges_view()`` to avoid copying a
snapshot; do not modify that vertex's edges while iterating the borrowed list.

``TSPSolver`` uses dense Prim followed by an iterative preorder traversal, requiring
O(n^2) time and O(n) auxiliary memory. Tours remain closed and satisfy the metric
two-approximation bound. Equal-distance ties are deterministic, so tour order may
differ from earlier releases. Storage, adjacency, and distance calculations can be
tuned directly in these implementations.

=======================
Building for Emscripten
=======================

To compile the emscripten library, first download, install and activate it using the following commands:

..  code-block:: shell

    git clone https://github.com/emscripten-core/emsdk.git
    cd emsdk
    ./emsdk install latest
    ./emsdk activate latest

Now you can compile OpenCAMLib like this (make sure to replace the ``path/to/`` sections):

..  code-block:: shell

    source path/to/emsdk/emsdk_env.sh
    git clone https://github.com/aewallin/opencamlib
    cd opencamlib
    mkdir build
    cd build
    emcmake cmake \
      -D CMAKE_BUILD_TYPE="Release" \
      -D BUILD_EMSCRIPTEN_LIB="ON" \
      -D USE_OPENMP="OFF" \
      -D CMAKE_INSTALL_PREFIX="/path/to/opencamlib/src/npmpackage/build" \
      ..
    emmake make # try emmake make -j4 for a faster build if you have a multi-core machine

Note that ``USE_OPENMP`` has been turned off, OpenMP is not supported with Emscripten at the moment

====================
Building for Node.js
====================

To compile the Node.js library, install the dependencies in ``src/nodejslib``:

..  code-block:: shell

    cd src/nodejslib
    npm install

Next, use cmake-js to compile the library:

..  code-block:: shell

    git clone https://github.com/aewallin/opencamlib
    cd opencamlib
    mkdir build
    cd build
    ../src/nodejslib/node_modules/.bin/cmake-js \
      build \
      --directory ".." \
      --out "." \
      --parallel 4 \
      --CD BUILD_NODEJS_LIB="ON" \
      --CD USE_OPENMP="ON" \
      --CD CMAKE_INSTALL_PREFIX="/path/to/opencamlib/build/Release/$(node --print 'process.platform')-nodejs-$(node --print 'process.arch')" \
      --config "Release"

===================
Building for Python
===================

The Python bindings require Python 3.8 or later, use pybind11, and can be built and installed directly with pip.
From the repository root, run:

..  code-block:: shell

    python -m pip install .

pip creates an isolated build environment and installs scikit-build-core and pybind11 as declared build dependencies.
To install the base dependencies and build the Python package:

..  code-block:: shell

    ./install.sh --build-library python --install-system-deps --python-pip-install

The package includes a ``py.typed`` marker and type information for the ``opencamlib.ocl`` extension, so Python type checkers and editors can use the bundled API signatures.
To generate the type stubs, you can use the ``pybind11-stubgen`` tool:

..  code-block:: shell

    python -m pip install pybind11-stubgen
    pybind11-stubgen opencamlib -o src/pythonlib/

Python subclasses of ``MillingCutter`` no longer provide callbacks to native
operations. Use the built-in cutters or implement new cutter behavior in C++.

*****
Usage
*****

Please take a look at the ``examples/`` folder on how to use OpenCAMLib.
For each language there is an example named ``test`` which calls all of the algorithms.

There is also some API documentation over here: https://opencamlib.readthedocs.io

***************
Cross Compiling
***************

To compile OpenCAMLib for other architectures, we recommend the following strategies.

=====
macOS
=====

Cross compiling on macOS is possible by setting the CMake ``CMAKE_OSX_ARCHITECTURES`` flag.
When using the ``install.sh`` script, you can use the ``--macos-architecture`` flag to accomplish the same thing.
Make sure to take a look at the other ``--*-architecture`` flags when cross compiling.

=======
Windows
=======

Cross compiling on Windows is possible by using the "Visual Studio" generator (default) and by setting the CMake ``CMAKE_GENERATOR_PLATFORM`` flag.
When using the ``install.sh`` script, you can use the ``--cmake-generator-platform`` flag to accomplish the same thing.
Make sure to take a look at the other ``--*-architecture`` flags when cross compiling.

=====
Linux
=====

To ensure that compiled libraries work on older linux versions, it has to be compiled with an older Glibc version.
The easiest way to accomplish this is by using Docker, there are images available especially for this purpose.
When using the ``install.sh`` script, you can use the ``--docker-image`` flag which will make the command run in a container with the given image name.

C++ docker image
----------------

When cross compiling the C++ library, make sure to use an old Glibc, this is included in the dockcross docker images.
For a list of supported architectures, take a look at:

https://github.com/dockcross/dockcross#summary-cross-compilers

Node.js docker image
--------------------

Cross compilers for node.js are available here:

https://github.com/prebuild/docker-images

Python docker image
-------------------

Cross compilers for python are here:

https://github.com/pypa/manylinux#manylinux2014-centos-7-based

*****
Links
*****

- repository https://github.com/aewallin/opencamlib
- PPAs
  - https://launchpad.net/~iacobs/+archive/ubuntu/cnc/
  - https://launchpad.net/~neomilium/+archive/ubuntu/cam
  - https://launchpad.net/~freecad-community/+archive/ubuntu/ppa
  - (updated 2012) https://launchpad.net/~anders-e-e-wallin/+archive/ubuntu/cam
- mailing-list http://groups.google.com/group/opencamlib
- IRC-channel #cam on irc.freenode.net
- coding standard (?) http://www.possibility.com/Cpp/CppCodingStandard.html

*********************
Organization of Files
*********************

(generate this with 'tree -dL 2')::

 ├── docs                        documentation (not much here yet!)
 ├── examples                    c++, emscripten, nodejs and python examples
 ├── scripts                     CI scripts for installing and building ocl
 ├── src
 │   ├── algo                    algorithms under development
 │   ├── common                  common algorithms and data-structures
 │   ├── cutters                 cutter-classes
 │   ├── cxxlib                  c++ library cmake config
 │   ├── deb                     debian package cmake config
 │   ├── dropcutter              drop-cutter algorithms and operations
 │   ├── emscriptenlib           bindings for emscripten library
 │   ├── geo                     primitive geometry classes (point, triangle, stlsurf, etc.)
 │   ├── nodejslib               Node.js library bindings and cmake config
 │   ├── npmpackage              combined Node.js and emscripten wrappers, for publishing to npm
 │   ├── pythonlib               python library bindings and cmake config
 └── stl                         STL files for testing

=================
Cutter benchmarks
=================

An optional native benchmark separates KD-tree setup from toolpath execution::

    cmake -S . -B build/bench -DCMAKE_BUILD_TYPE=Release -DBUILD_CXX_LIB=ON -DBUILD_PY_LIB=OFF -DBUILD_BENCHMARKS=ON -DUSE_OPENMP=ON
    cmake --build build/bench --target cutter_benchmark
    OMP_NUM_THREADS=4 build/bench/cutter_benchmark stl/gnu_tux_mod.stl waterline 4 5

The arguments are the STL file, operation, thread count, and repetition count.
Operations are ``waterline``, ``adaptive-waterline``, ``dropcutter``, and
``adaptive-dropcutter``. The fixture uses a diameter-4 cylindrical cutter,
length 20, sampling 0.1, waterline Z=1, and 25 horizontal drop-cutter spans
at Z=-1 across the model bounds. Adaptive operations retain their default
minimum sampling settings.

The final JSON line reports average ``setup_ms``, ``run_ms``, and ``total_ms``;
STL loading is excluded, while total time includes operation construction,
configuration, output collection, and destruction. Point counts and coordinate
checksums help detect changed workloads; use the geometry and cutter tests for
correctness checks. Compare identical compiler settings, models, and thread
counts, and alternate old/new runs to reduce timing bias. Set ``OMP_NUM_THREADS``
too when comparing revisions whose adaptive waterline ignored ``setThreads``.

OpenMP is optional. With it enabled, batch operations distribute individual
points/fibers, adaptive path drop-cutter distributes independent spans while
preserving path order, and adaptive waterline uses at most two threads (one per
fiber direction). ``setThreads(1)`` requests serial execution. A single adaptive
path span remains serial.

To collect alternating runs from two builds and save the raw samples and
medians::

    python3 benchmarks/compare.py build/before/cutter_benchmark build/after/cutter_benchmark stl/gnu_tux_mod.stl stl/sphere_on_plate.stl --threads 1 4 --repetitions 5 --output build/comparison.json
