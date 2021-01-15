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
- PPAs
 - https://launchpad.net/~iacobs/+archive/ubuntu/cnc/ 
 - https://launchpad.net/~neomilium/+archive/ubuntu/cam
 - https://launchpad.net/~freecad-community/+archive/ubuntu/ppa 
 - (updated 2012) https://launchpad.net/~anders-e-e-wallin/+archive/ubuntu/cam
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
 $ sudo apt install cmake
 $ sudo apt install libboost-program-options-dev
 $ sudo apt install doxygen
 $ sudo apt install texlive-full
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

Emscripten
----------


$ cd src/emscriptenlib
$ ./build.sh

macOS
-----

Make sure you have XCode installed and that it is correctly set:

$ sudo xcode-select --switch /Applications/Xcode.app/Contents/Developer

Install homebrew (https://brew.sh) and use it to install the dependencies:

$ brew install boost boost-python3 doxygen libomp

JavaScript
----------

opencamlib is compiled to wasm using emscripten.
You can simply use the package from npm which works in the browser and in node.js

$ npm install --save-dev opencamlib

If you want to build it yourself, make sure to clone emscripten in the parent folder of the opencamlib project.

$ git clone https://github.com/emscripten-core/emsdk.git
$ cd emsdk
$ ./emsdk install latest
$ ./emsdk activate latest

Now your can compile the emscripten.wasm and emscripten.js files using:

$ cd src/emscriptenlib
$ ./build.sh

The opencamlib.wasm file will be copied to src/npmpackage

To work on the npm package, run

$ npm install
$ npm start

npm install

        ./node_modules/.bin/cmake-js compile --arch=x64 \
          -D USE_VERSION_AND_PLATFORM_SUFFIX="ON" \
          -D BUILD_CXX_LIB="$BUILD_CXX_LIB" \
          -D BUILD_NODEJS_LIB="$BUILD_NODEJS_LIB" \
          -D BUILD_PY_LIB="$BUILD_PY_LIB" \
          -D USE_PY_3="$USE_PY_3" \
          -D VERSION_STRING="$TRAVIS_BRANCH"

        /usr/local/bin/cmake \
          -D USE_VERSION_AND_PLATFORM_SUFFIX="ON" \
          -D BUILD_CXX_LIB="$BUILD_CXX_LIB" \
          -D BUILD_NODEJS_LIB="$BUILD_NODEJS_LIB" \
          -D BUILD_PY_LIB="$BUILD_PY_LIB" \
          -D USE_PY_3="$USE_PY_3" \
          -D VERSION_STRING="$TRAVIS_BRANCH" \
          ../src;

      if [[ "$USE_PY_3" == "ON" ]]; then
        otool -L /usr/local/Cellar/python/3.7.2_1/Frameworks/Python.framework/Versions/3.7/lib/python3.7/site-packages/ocl.so;
      else
        otool -L /usr/local/Cellar/python@2/2.7.15_1/Frameworks/Python.framework/Versions/2.7/lib/python2.7/site-packages/ocl.so;
      fi

DOCKER BUILD
-----------------------

To run this in Docker, simply run::  

 $ docker build . -t opencamlib
 $ docker run -it -p 5900:5900 -e VNC_SERVER_PASSWORD=TestVNC --user cam --privileged opencamlib

Then simply open up a VNC socket connection. With Mac, you can just open Finder, then run CMD + K. 

Type in "vnc://localhost:5900" as the server address, and type "TestVNC" as the password. 

Right click on the desktop terminal that shows up and you can open up a terminal inside there. This is a easy and quick way to get started with opencamlib, while still maintaining all the functionality
