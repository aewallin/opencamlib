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

========================
Multithreaded WebAssembly
========================

The `Tux toolpath demo <examples/web-demo/README.rst>`_ builds a single HTML file
containing the model and serial/threaded WASM runtimes, with Three.js loaded from
a CDN. Build it using the documented presets, or download the
``opencamlib-web-demo`` artifact from a successful Test workflow run. Open the
HTML file directly for serial execution, or run
``python3 examples/web-demo/serve.py`` and open http://localhost:8001 to enable
threads. It includes STL uploads, cutter and sampling presets, timing statistics,
playback, and point export. The generated HTML stays out of source control.
The Test workflow also supports opt-in GitHub Pages publishing; see the demo
instructions for setup. Pages runs the serial build in a background worker.

For fast Release WASM builds, check that configuration reports interprocedural
optimization enabled (``OCL_ENABLE_IPO=ON``). CMake reports a warning if its
compiler check fails. ``OCL_WASM_NATIVE_EXCEPTIONS=ON`` also reduces exception
wrapper overhead in supporting WASM runtimes; it is enabled in the web demo.
This option defaults to OFF to retain compatibility with older runtimes.
See the demo instructions for a repeatable adaptive-waterline benchmark.

The Emscripten build supports an optional pthread backend. Enable it with::

    ./install.sh --build-library emscripten --build-type release --emscripten-threads --install

Or configure a separate build after activating the Emscripten SDK::

    emcmake cmake -S . -B build/wasm-threads -DCMAKE_BUILD_TYPE=Release \
      -DBUILD_EMSCRIPTEN_LIB=ON -DBUILD_DOC=OFF -DUSE_OPENMP=OFF \
      -DOCL_WASM_THREADS=ON -DOCL_WASM_THREAD_POOL_SIZE=3
    cmake --build build/wasm-threads --parallel
    ctest --test-dir build/wasm-threads --output-on-failure

``OCL_WASM_THREAD_POOL_SIZE`` counts preloaded workers in addition to the calling
thread: the default of 3 supports up to 4 threads per operation. The installer
also accepts this setting as an environment variable. ``setThreads(1)`` runs
serially; zero also means one. Larger requests are capped to the pool size plus
one. Defaults use the smaller of this limit and the reported CPU count.
``module.maxThreads()`` reports the build's limit, and operations expose
``setThreads`` and ``getThreads`` in both Embind and the browser package.

Batch drop/push-cutter work is distributed by point/fiber. Adaptive path
drop-cutter distributes independent spans and preserves output order; a single
span stays serial. Adaptive waterline batches independent midpoint fibers
across both directions using the selected threads, then restores the original
fiber order before serial contour assembly. Native builds retain their OpenMP
backend.

Serve the page and assets over HTTPS (or localhost), with these response headers::

    Cross-Origin-Opener-Policy: same-origin
    Cross-Origin-Embedder-Policy: require-corp

The page must report ``crossOriginIsolated === true``. External assets must also
satisfy the embedding policy. The threaded module uses SharedArrayBuffer and
cannot fall back on browsers without it; keep a separate build with
``OCL_WASM_THREADS=OFF`` for those deployments. See the
`Emscripten pthread documentation <https://emscripten.org/docs/porting/pthreads.html>`_.

The generated ``ocl.js`` remains a single file. Load it directly to obtain the
Embind factory, or install it into ``src/npmpackage/build`` and run
``npm run build-emscripten`` there to build the browser package. When bundling,
also serve the original generated ``ocl.js`` as a separate asset and configure
its absolute URL **before constructing any operations**::

    await ocl.initialize({
        mainScriptUrlOrBlob: new URL('/assets/ocl.js', location.href).href
    })
    const operation = new ocl.PathDropCutter()
    await operation.setThreads(4)
    // Configure surface, cutter, path, sampling and Z, then run and dispose.

For direct Embind use, call ``await ocl()`` after loading the generated script;
when its URL cannot be inferred, pass the same ``mainScriptUrlOrBlob`` option to
that factory. The default environment supports both pages and Web Workers. Add
``-DOCL_WASM_ENVIRONMENT=web,worker,node`` to also run the module under Node.

The worker pool is ready when initialization resolves. Toolpath calls remain
synchronous inside WASM, even when the browser wrapper returns a promise. Run
the module and operations in a dedicated Web Worker to keep the page responsive,
and send inputs/results using messages. The pthread workers share that module's
memory; do not mutate or dispose an operation or its inputs during ``run()``.

``ctest`` runs real WASM concurrency and cutter-equivalence tests under Node.
To test the browser package on both the page and a dedicated worker with a
temporary Chrome profile and an isolated local server::

    python3 tests/wasm_browser.py build/wasm-threads/ocl.js src/npmpackage/browser.js

Use ``--chrome /path/to/chrome`` or ``CHROME_BIN`` if Chrome is not on PATH.

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

An optional fifth argument selects ``cyl`` (the default), ``ball``, ``bull``,
``cone``, or ``cyl-cone``. The ball, bull, and cone cutters also have diameter 4
and length 20; the bull has corner radius 1 and the cone angle is 0.7 radians.
The composite cutter uses ``CylConeCutter(2, 4, 0.7)``. Pass ``--cutter cyl-cone``
to ``compare.py`` to compare this workload with the same cutter in both builds.

The final JSON line reports average ``setup_ms``, ``run_ms``, and ``total_ms``;
STL loading is excluded, while total time includes operation construction,
configuration, output collection, and destruction. Point counts and coordinate
checksums help detect changed workloads; use the geometry and cutter tests for
correctness checks. Compare identical compiler settings, models, and thread
counts, and alternate old/new runs to reduce timing bias. Set ``OMP_NUM_THREADS``
too when comparing revisions whose adaptive waterline ignored ``setThreads``.

OpenMP is optional. With it enabled, batch operations distribute individual
points/fibers, adaptive path drop-cutter distributes independent spans while
preserving path order, and adaptive waterline batches independent refinement
fibers across both directions. Contour assembly remains serial.
``setThreads(1)`` requests serial execution. A single adaptive path span remains
serial.

To collect alternating runs from two builds and save the raw samples and
medians::

    python3 benchmarks/compare.py build/before/cutter_benchmark build/after/cutter_benchmark stl/gnu_tux_mod.stl stl/sphere_on_plate.stl --threads 1 4 --repetitions 5 --output build/comparison.json


==================
Development checks
==================

With CMake 3.21+ and Ninja installed, use the same configurations as CI::

    cmake --preset debug
    cmake --build --preset debug
    ctest --preset debug

Other presets are ``release``, ``release-openmp``, and ``sanitizers``. The
sanitizer preset enables AddressSanitizer, UndefinedBehaviorSanitizer, and
floating-point-to-integer conversion checks with GCC or Clang. Linux CI also
enables leak detection. Debug and sanitizer builds retain debug symbols and
disable interprocedural optimization. Sanitizer builds use ``-O1``.
Manual CMake configuration still supports CMake 3.18+.
``OCL_ENABLE_IPO=OFF`` disables optimization across translation units.

Core sources are compiled once into ``ocl_core`` and reused by the selected
binding, tests, and benchmarks. Configure each binding in its own build directory.

The binding contract uses ``tests/fixtures/path_contract.txt`` for C++, Python,
Node, and WebAssembly. It checks complete, ordered line/arc paths, minimum Z,
repeatability, input lifetimes, and invalid inputs. To run the native reference::

    build/debug/binding_fixture tests/fixtures/path_contract.txt > build/cxx.jsonl

``tests/bindings.py [python-module-directory]`` and
``tests/bindings.js napi`` emit the same format; the Node runner uses
``OCL_NODE_PATH`` and requires ``npm run build-node`` in ``src/npmpackage``.
``tests/bindings.js emscripten path/to/ocl.js`` accepts a WASM build configured
with ``OCL_WASM_ENVIRONMENT=web,node``. Compare the outputs with
``tests/compare_bindings.py``. The CI workflow contains a complete example.

======================
Behavior and lifetimes
======================

* ``setZ`` sets the minimum Z for both path drop-cutter algorithms, including
  adaptive samples. Input path Z does not override this floor.
* Repeated path drop-cutter and waterline ``run()`` calls replace previous
  results. Waterline ``reset()`` also clears its output loops.
* Sampling intervals must be finite and positive. Missing surfaces, cutters,
  or paths produce exceptions. Empty surfaces are valid: drop-cutter returns
  the floor, while waterline returns no loops.
* ``Path`` owns spans created by ``append``, supports deep copying and moving,
  and retains the public ``span_list`` as a non-owning view. Do not manually
  delete spans created by ``append``. Directly inserted pointers remain owned
  by the caller. Custom spans implement ``clone()`` to support path copying.
* Operations own their internal trees and sub-operations and cannot be copied.
  C++ callers keep their surface, cutter, and path alive until the operation
  is finished. Python and Node bindings retain these inputs automatically.
  Call ``setSTL`` again after modifying a surface. These ownership/layout changes
  require rebuilding C++ consumers against the updated library.
* The native STL reader accepts ASCII and binary STL, including binary headers
  beginning with ``solid``. It rejects unreadable, truncated, malformed, and
  degenerate geometry with exceptions. Parsing is transactional: invalid files
  do not partially append triangles. ASCII coordinates retain double precision;
  older versions rounded them to float precision.

Both JavaScript backends support lines and arcs and expose ``getCLPoints()``
with ``getPoints()`` as an alias. Node methods are synchronous; browser methods
return promises. Using ``await`` works with either backend. Browser operations
can be constructed and called before the runtime is ready: each operation
waits on one initialization promise, and calls execute in invocation order.
Initialization failures propagate to every call; a rejected parameter change
does not prevent later calls. Once initialized, the native calls themselves
are synchronous, including ``run()``. Promises alone do not move computation
off the calling thread; use a Web Worker, as the demo does, for a responsive UI.

Browser operations must be disposed to release their WASM objects; Node
operations also offer explicit disposal. Disposal waits for previously
requested calls, is idempotent, and rejects further use immediately::

    const operation = new ocl.AdaptivePathDropCutter()
    try {
        await operation.setSTL(surface)
        await operation.setCutter(cutter)
        await operation.setPath(path)
        await operation.setSampling(0.1)
        await operation.setZ(-1)
        await operation.run()
        const points = await operation.getCLPoints()
    } finally {
        await operation.dispose()
    }

KD-trees retain immutable contiguous triangle snapshots. Waterline shares one
snapshot between its directional indexes. C++ users building multiple indexes
can explicitly reuse a ``KDTree<Triangle>::Snapshot`` with ``buildSnapshot``;
the tree retains ownership, and the caller must not mutate the backing vector.
Stable partitioning preserves candidate order and equal-height contact choices.
When comparing performance across STL reader versions, use the same binary STL
inputs so ASCII precision changes do not change the workload.
