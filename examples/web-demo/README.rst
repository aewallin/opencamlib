Tux toolpath demo
================

Build ``index.html`` as described below, or download the
``opencamlib-web-demo`` artifact from a successful Test workflow run. Open it
in a current browser. The Tux STL and both OpenCAMLib WASM
builds are embedded in this single file. Only Three.js and its addons are fetched
from jsDelivr (pinned to 0.180.0), so an internet connection is required.

For multithreading, start the included local server from the repository root::

    python3 examples/web-demo/serve.py

Then open http://localhost:8001. The server supplies the COOP/COEP headers needed
for SharedArrayBuffer. Opening the file directly, or serving it without those
headers, automatically selects the serial build. Both modes calculate in a
background Web Worker, leaving the viewer responsive.

Publishing to GitHub Pages
--------------------------

The Test workflow can publish the generated HTML directly to GitHub Pages.
It reuses the ``opencamlib-web-demo`` artifact after the core checks and binding
contract job (including the browser regression) pass. No generated HTML needs
to be committed. Deployment uploads only ``index.html`` and serves the demo at
the site's root, normally ``https://<owner>.github.io/<repository>/``.

To enable publishing in a repository:

1. In **Settings → Pages → Build and deployment**, set **Source** to
   **GitHub Actions**. See GitHub's `publishing source instructions
   <https://docs.github.com/en/pages/getting-started-with-github-pages/configuring-a-publishing-source-for-your-github-pages-site>`_.
2. In **Settings → Secrets and variables → Actions → Variables**, create a
   repository variable named ``OCL_PUBLISH_DEMO`` with the value ``true``.
   This opts the repository into using its Pages site for the demo; it replaces
   any existing site content on deployment.
3. Merge the workflow and demo sources into the default branch. Pushes to the
   default branch (``master`` here) publish after the required checks pass.
   Alternatively, run **Actions → Test → Run workflow** on the default branch.
   If the repository uses a different default branch, include it in the Test
   workflow's push trigger. Pull requests, tags, and other branches do not deploy.

The **Publish demo to GitHub Pages** job reports the published URL through the
``github-pages`` environment. Removing the variable or setting it to ``false``
stops future deployments without taking down the current site. Repositories
that leave publishing disabled still build, test, and upload the demo artifact.

GitHub Pages does not currently let a site set the COOP/COEP response headers
required for SharedArrayBuffer; see GitHub's `header support discussion
<https://github.com/orgs/community/discussions/13309>`_. The single-file demo
therefore uses its serial WASM build on Pages, in a background worker. For
multiple compute threads, run the included local server or use a host that
can supply those headers. No service worker or additional hosting files are
required for the Pages deployment.

Using the demo
--------------

Use the included Tux model, or open an ASCII or binary STL (up to 100 MB).
The model selector lets you switch between Tux and the last uploaded file.
Uploads stay on the device. Invalid/empty meshes, non-finite vertices, and
zero-area triangles are rejected before replacing the current model. Coordinates
are preserved; the camera, grid, pass spacing, and initial Z settings fit
the new model. Uploading an STL sets Start Z to its exact highest Z, preserving
the original coordinate offset. The Model top and Top − step shortcuts set
Start Z to that height or one current step down below it. The exact top may
have no contour, particularly at a pointed summit; use Top − step to start
below it. Tux keeps its original Z = 2 mm preview default. Changing models
clears the previous toolpath.

Sampling starts with the Fine preset, using fixed distances in millimetres:

=================  ==============  ================
Preset             Sample spacing  Minimum spacing
=================  ==============  ================
Preview            0.25 mm         0.05 mm
Standard           0.10 mm         0.01 mm
Fine (default)     0.05 mm         0.005 mm
Very fine          0.02 mm         0.002 mm
=================  ==============  ================

The buttons set both values without starting a calculation. Manual values
remain editable, and the matching button is highlighted (none for custom
values). Loading another model, switching back to Tux, or changing cutters
preserves the current sampling values. Larger STLs no longer silently coarsen
sampling using the former model-size/80 and model-size/300 rules.

These presets offer progressively finer sampling for CAM work; their numbers
are sampling distances, not certified machining tolerances. Uniform methods
use sample spacing for their sampling grid; adaptive methods subdivide around
changes in contact geometry toward minimum spacing. Minimum spacing is a
refinement cutoff, not a strict lower bound on output point distance; binary
subdivision can go below it. Smaller values generally add computation and
output points. Actual machining accuracy also depends on STL approximation,
the geometry between samples, pass spacing, step down, and the machine/tool.
The presets do not set pass spacing or step down. A chordal-error guarantee
would require a separate geometric error check; reducing these two distances
alone does not provide one. See the distinction between sampling intervals in
the `OpenCAMLib operation API
<https://opencamlib.readthedocs.io/en/stable/operations.html>`_ and toolpath
deviation tolerance in `Autodesk's tolerance reference
<https://help.autodesk.com/view/fusion360/ENU/?contextId=MFG-REF-3D-STEEP-SHALLOW-SMOOTHING>`_.

The demo includes uniform/adaptive drop-cutter and waterline operations, ball,
flat, bull, and conical cutters, editable cutter presets, sampling controls,
thread selection, camera presets, visibility controls, cutter-tip playback,
and JSON point export. Presets are a 0.8 mm ball nose, a 1 mm flat end mill,
a 1 mm bull nose with a 0.2 mm corner radius, and a 1 mm V-bit with a 60 degree
included angle. Preset lengths cover the loaded model down to the initial
minimum Z (at least 10 mm). Editing a cutter dimension switches the preset to
Custom, and loading another model preserves custom dimensions.
All millimetre inputs have up/down buttons and keyboard arrows that change
the current value by 0.1 mm. Finer values such as 0.04 mm remain valid when
typed; decrementing stops before crossing the field's minimum.
Drop-cutter uses alternating raster passes. Waterline starts at the selected Z
and steps downward for the chosen number of levels. The initial waterline
preview uses one level; increase Z levels for a deeper study. The former
20-level maximum was a demo input restriction, not an engine limit, and has
been removed. Counts must be positive safe integers and the resulting Z range
must be finite; runtime and memory use grow with the requested work. A viewer overlay
shows the current stage, elapsed time, and a cancel button. Waterline progress
counts completed levels; drop-cutter uses an indeterminate indicator.
Exported paths are separate segments; the viewer does not join passes or
contours with cutting moves. Exports include the selected model's filename
and individual waterline level timings.

Waterline cutter reach
---------------------

The demo requires the cutter to reach the highest model Z from the lowest
requested waterline Z: ``length >= model.maxZ - lowestZ``. This is a conservative
precondition for these surface-based operations, not a full solid-volume or
tool-holder collision check. With a shorter cutter, upper surface contacts
can be omitted and material inside the part can appear as false interior
contours. The demo rejects that configuration and reports the required length,
rounded up to 0.1 mm, rather than silently extending a custom cutter or removing
inner contours (which could be legitimate cavities).

Statistics and browser checks
-----------------------------

Compute time measures the native ``run()`` calls. The footer includes setup and
output extraction. Path length excludes transfers between separate paths.
The cutter preview follows sample indices, not feed rate, and displays the full
length of the cutter used for the last generated path, from its tip upward
along +Z. Regenerate after editing cutter dimensions to update the preview.
Fit and the camera presets include the visible cutter. Length consistently
means total tip-to-top length for all cutter types; the demo subtracts the
conical tip height when passing the shaft-length argument to ConeCutter.
The live readout shows X/Y/Z, the current pass or contour, and
the point index during playback or scrubbing, even when the cutter mesh is
hidden. Waterline paths also show the Z-level index. Coordinates are rounded
to three decimals in the readout; hover over a value for full precision.
Model coordinates are treated as millimeters (STL has no units).

After building the HTML, run the browser regression from the repository root
with Node 22+ and Chrome installed::

    node tests/web_demo.mjs

Set ``CHROME_BIN`` to override the browser executable. The test covers decimal
steppers, precise input, model-top shortcuts, sampling presets and persistence,
full cutter geometry and camera fitting, cutter reach, presets, uploads, multi-level contours,
tip readouts, cancellation, exports, and threaded/serial loading. All upload
fixtures are generated by the test. Screenshots are written to ``build/web-demo``;
set ``OCL_WEB_DEMO_ARTIFACTS`` to choose another directory. CI builds the demo,
runs this regression, and uploads the HTML and browser screenshots as artifacts.

Adaptive waterline performance
------------------------------

Adaptive waterline batches independent midpoint fibers across both directions
using the selected threads. Subdivision decisions and contact ordering are
preserved, and the fibers are sorted before serial contour assembly. Serial
execution uses the original recursive traversal. Parallel refinement retains
intermediate fibers until sampling completes, which uses more temporary memory.

The demo presets enable Release optimization, interprocedural optimization
(``OCL_ENABLE_IPO``), and native WASM exceptions
(``OCL_WASM_NATIVE_EXCEPTIONS``). Check that configuration reports
``Interprocedural optimization enabled``. Native exceptions require a supporting
WASM engine; general library builds retain JavaScript exception handling by
default. Emscripten documents `link-time optimization
<https://emscripten.org/docs/optimizing/Optimizing-Code.html#lto>`_ and its
`exception handling modes <https://emscripten.org/docs/porting/exceptions.html>`_.

For a repeatable workload and ordered-coordinate hash, run::

    node benchmarks/wasm_waterline.js build/web-demo-threaded/ocl.js stl/gnu_tux_mod.stl 4 3

This benchmark uses a 0.8 mm ball nose, 0.15/0.04 mm sampling, and five Z levels
from 2 to 0 mm. Its sampling settings are fixed for comparisons and differ from
the demo's Fine default. Use identical STL bytes, compiler settings, and thread
counts when comparing builds. The demo converts ASCII geometry to float32 for
Three.js and WASM, while the native STL reader retains double precision, so a
benchmark loading ASCII directly may not match the demo's coordinates exactly.

Building the HTML
-----------------

Install CMake 3.21+, Ninja, Python 3, and activate the Emscripten SDK (CI uses
4.0.22). From the repository root::

    emcmake cmake --preset web-demo-serial
    cmake --build --preset web-demo-serial --parallel
    ctest --preset web-demo-serial

    emcmake cmake --preset web-demo-threaded
    cmake --build --preset web-demo-threaded --parallel
    ctest --preset web-demo-threaded

    python3 examples/web-demo/build.py
    python3 examples/web-demo/serve.py

The presets include Node support for tests and benchmarks. The threaded build
preloads three workers, supporting up to four threads including the caller.
``build.py`` defaults to those two preset output directories. To package other
compatible SINGLE_FILE runtime builds, pass their paths explicitly::

    python3 examples/web-demo/build.py --serial path/to/serial/ocl.js \
      --threaded path/to/threaded/ocl.js --output build/demo/index.html

Edit ``demo.html.in`` and rerun ``build.py`` after source changes. The script
converts the repository's ASCII Tux STL to binary float32 geometry and embeds
reproducible gzip/base64 payloads. The viewer and CAM engine use the same
geometry. Only the resulting ``index.html`` is needed to distribute the demo;
it is generated locally or by CI and is excluded from Git. OpenCAMLib's source
and LGPL license accompany this repository.
