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

- The Python library is called ``opencamlib`` and is hosted on PyPi (pypi.org), precompiled libraries are available for Python v3.7 up to v3.11.
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

OpenCAMLib uses functionality from a library called Boost.
For the Python library it uses an extra library called Boost.Python.

Only the Python bindings need Boost to be **compiled** (with Boost.Python).
All other libraries **DO NOT** need Boost to be compiled, in those cases, a headers only version will suffice.
So, if you are not compiling the Python libraries, simply download Boost, extract it into a folder, and tell CMake where to look for it.

Make sure to download Boost from the boost.org downloads page, if you download it from github, you have to make sure to install the git submodules **and** build the headers.

We provide a ``install.sh`` script that helps with installation of dependencies and building OpenCAMLib libraries, you might want to take a look at it first.
You can run ``./install.sh --help`` to look at the available options, or inspect it's source code to find out more.

============
Dependencies
============

To compile OpenCAMLib, you need:

- **C++ compiler** (It should at least support C++ 14)
- **Git** (This is used for cloning the repository, and the emscripten SDK)
- **CMake** (At least version 3.15)
- **Boost** (When compiling the Python library, you have to **compile** Boost.Python for your Python version after installation)

At this time of writing, here are the packages to install:

Ubuntu Dependencies
-------------------

..  code-block:: shell

    sudo apt install -y git cmake curl build-essential libboost-dev

macOS Dependencies
------------------

..  code-block:: shell

    brew install git cmake curl boost python@3.11 boost-python3

Windows Dependencies
--------------------

Install

- Visual Studio Build Tools (https://visualstudio.microsoft.com/visual-cpp-build-tools/)
- Git (https://git-scm.com/download/win)
- CMake (https://git-scm.com/download/win)

By downloading the installers from the internet, or by using your package manager.

================
Building for C++
================

The C++ library is the easiest to build, it only depends on Boost's headers.
Make sure you have a compiler, git, cmake and Boost installed (or simply download and extract it somewhere).

..  code-block:: shell

    git clone https://github.com/aewallin/opencamlib
    cd opencamlib
    mkdir build
    cd build
    cmake .. -D CXX_LIB="ON"
    make . # try make -j4 for a faster build if you have a multi-core machine
    make install .

When boost is not in a standard location, you can add the ``-D BOOST_ROOT=/path/to/boost`` option to the cmake command.

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
      -D BOOST_ROOT="/path/to/boost" \
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
      --CD BOOST_ROOT="/path/to/boost" \
      --config "Release"

===================
Building for Python
===================

The Python library can be compiled similarly to the C++ example above, however, this time Boost.Python has to be compiled first.
Most systems have Boost.Python available as a download, but only for a specific Python version only (usually the latest Python version).
These might work if you are using Python from the same package provider, but, unfortunately, this is not a very reliable method, so compiling them yourself is usually the best option.

First, download and extract Boost:

..  code-block:: shell

    curl "https://boostorg.jfrog.io/artifactory/main/release/1.80.0/source/boost_1_80_0.tar.gz" --output "boost_1_80_0.tar.gz" --location
    tar -zxf boost_1_80_0.tar.gz -C /tmp/boost
    cd /tmp/boost/boost_1_80_0

Now we can compile it:

..  code-block:: shell

    echo "using python ;" > ./user-config.jam
    ./bootstrap.sh
    ./b2 \
      -a \
      threading="multi" \
      -j4 \
      variant="release" \
      link="static" \
      address-model="64" \
      architecture="x86" \
      --layout="system" \
      --with-python \
      --user-config="./user-config.jam" \
      cxxflags="-fPIC" \
      stage

Note that you can customize the user-config.jam file to point it to your Python installation
(see: https://www.boost.org/doc/libs/1_78_0/libs/python/doc/html/building/configuring_boost_build.html).
You should also specify the correct architecture and address-model.
On windows, make sure to use windows style paths, e.g. ``C:\\path\\to\\Python``

*****
Usage
*****

Please take a look at the ``examples/`` folder on how to use OpenCAMLib.
For each language there is an example named ``test`` which calls all of the algorithms.

There is also some API documentation over here: https://opencamlib.readthedocs.io

***************
Common Problems
***************

Compiling OpenCAMLib is unfortunately not very easy and there are many things that can go wrong.
Here is a list of common problems and solutions.

=================================================
Could NOT find Boost (missing: Boost_INCLUDE_DIR)
=================================================

This happens a lot, here are some of the reasons why this happens:

**You don't have Boost installed.**

If you forgot to install boost, go ahead and download Boost from from their website: https://www.boost.org/users/download/ and extract it somewhere.
Now, when compiling the C++ or node.js module, add the

``-D BOOST_ROOT=/path/to/extracted/boost`` flag to the ``cmake ..`` command, or the.

``--boost-prefix /path/to/extracted/boost`` flag to the ``./install.sh`` command

**You installed Boost from Github.**

The boost that is hosted on Github does not have the headers yet! To compile those, you should run the following commands:

..  code-block:: shell

    ./bootstrap.sh
    ./b2 headers

**Your CMake version has a FindBoost module which is unaware of your Boost's version.**

The CMake module that looks for Boost, is usually not aware of the existence of the latest Boost versions.
You can help it by providing the version number of your Boost with the ``-D Boost_ADDITIONAL_VERSIONS="1.80.0"`` flag.
Make sure to change 1.80.0 with your version of Boost.

It can also be helpful to enable ``Boost_DEBUG`` in the CMake configuration.

***************
Cross Compiling
***************

To compile OpenCAMLib for other architectures, we recommend the following strategies.
Always make sure to compile Boost for the correct architecture as well!

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
