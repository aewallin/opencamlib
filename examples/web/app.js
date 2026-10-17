const $ = (id) => document.getElementById(id);
const number = (id) => Number($(id).value);
let worker,
  workerURL,
  libraryURL,
  stl,
  bounds,
  availableThreads = 1;
let uploadedSTL,
  currentSource = "tux",
  modelName = "gnu_tux_mod.stl",
  loadingModel = false;
let ready = false,
  busy = false,
  runStarted = 0,
  activeParams,
  lastResult,
  runTimer,
  engineGeneration = 0;
let THREE,
  STLLoader,
  controls,
  renderer,
  scene,
  camera,
  model,
  pathGroup,
  cutterMesh,
  geometry,
  grid;
let playing = false,
  playback = 0,
  lastFrame = 0;
const fmt = new Intl.NumberFormat("en-US");
const status = (message, spinning = false) => {
  $("status").textContent = message;
  $("status").classList.toggle("busy", spinning);
  $("loading-stage").textContent = message;
};
const showError = (message) => {
  $("error").textContent = message;
  $("error").hidden = false;
};
const clearError = () => {
  $("error").hidden = true;
};

async function inflate(id) {
  const bytes = Uint8Array.from(atob($(id).textContent.trim()), (c) =>
    c.charCodeAt(0),
  );
  return new Response(
    new Blob([bytes]).stream().pipeThrough(new DecompressionStream("gzip")),
  ).arrayBuffer();
}

function conditionalFields() {
  const adaptive = $("algorithm").value.startsWith("Adaptive");
  const isPath = $("algorithm").value.includes("Path");
  const circular = ["circles", "spiral"].includes($("pattern").value);
  for (const [id, visible] of [
    ["min-field", adaptive],
    ["path-fields", isPath],
    ["water-fields", !isPath],
    ["path-options", isPath],
    ["scan-field", isPath && !circular],
    ["circle-field", isPath && circular],
    ["spiral-field", isPath && $("pattern").value === "spiral"],
    ["weave-field", !isPath],
    ["filter-field", $("path-filter").value === "line"],
    ["radius-field", $("cutter").value === "BullCutter"],
    ["angle-field", $("cutter").value === "ConeCutter"],
  ]) {
    $(id).hidden = !visible;
    $(id)
      .querySelectorAll("input,select")
      .forEach((input) => {
        input.disabled = !visible || busy || loadingModel;
      });
  }
  $("algorithm-help").textContent = isPath
    ? adaptive
      ? "Follow the chosen path pattern, adding samples where the surface changes."
      : "Follow the chosen path pattern with evenly spaced samples."
    : adaptive
      ? "Refine contours around changes in shape using the selected sampling threads. Complex surfaces can take longer than uniform waterline."
      : "Trace contours at fixed Z levels with a uniform grid of fibers.";
  $("pattern-help").textContent = circular
    ? $("pattern").value === "spiral"
      ? "One continuous spiral between the chosen radii. Pass spacing sets the radial advance per revolution."
      : "Separate circles between the chosen radii, plus a center pass when inner radius is zero. Pass spacing sets the maximum radial gap."
    : "Separate passes cover the model bounds, with half a cutter diameter of margin.";
  $("fit-circles").disabled =
    !isPath || !circular || busy || loadingModel || !bounds;
  for (const id of ["height-model-top", "height-top-step"])
    $(id).disabled = isPath || busy || loadingModel || !bounds;
  syncSamplingPresets();
  syncDimensionSteppers();
}

// Fixed physical distances: importing a larger part must not coarsen sampling.
const samplingPresets = {
  preview: { label: "Preview", sampling: 0.25, minSampling: 0.05 },
  standard: { label: "Standard", sampling: 0.1, minSampling: 0.01 },
  fine: { label: "Fine", sampling: 0.05, minSampling: 0.005 },
  veryFine: { label: "Very fine", sampling: 0.02, minSampling: 0.002 },
};

function syncSamplingPresets() {
  const sampling = $("sampling").valueAsNumber,
    minimum = $("min-sampling").valueAsNumber;
  let name = "Custom";
  for (const button of $("sampling-presets").querySelectorAll("button")) {
    const preset = samplingPresets[button.dataset.preset];
    const selected =
      sampling === preset.sampling && minimum === preset.minSampling;
    button.setAttribute("aria-pressed", String(selected));
    button.disabled = busy || loadingModel || !ready;
    if (selected) name = preset.label;
  }
  $("sampling-help").textContent =
    `${name} sampling. ${$("algorithm").value.startsWith("Adaptive") ? "Adaptive refinement uses the minimum spacing where the shape changes." : "Uniform sampling uses sample spacing only; minimum spacing applies to adaptive methods."} Smaller values increase calculation time and output size.`;
}

function initSamplingPresets() {
  for (const [id, preset] of Object.entries(samplingPresets)) {
    const button = document.createElement("button");
    button.type = "button";
    button.className = "secondary";
    button.id = `sampling-${id}`;
    button.dataset.preset = id;
    button.setAttribute("aria-controls", "sampling min-sampling");
    button.setAttribute(
      "aria-label",
      `${preset.label}: sample spacing ${preset.sampling} mm, minimum spacing ${preset.minSampling} mm`,
    );
    button.append(preset.label);
    const values = document.createElement("small");
    values.textContent = `${preset.sampling} / ${preset.minSampling}`;
    button.append(values);
    button.addEventListener("click", () => {
      if (busy || loadingModel || !ready) return;
      for (const [input, value] of [
        [$("sampling"), preset.sampling],
        [$("min-sampling"), preset.minSampling],
      ]) {
        input.value = String(value);
        input.dispatchEvent(new Event("input", { bubbles: true }));
        input.dispatchEvent(new Event("change", { bubbles: true }));
      }
      clearError();
    });
    $("sampling-presets").append(button);
  }
  for (const id of ["sampling", "min-sampling"]) {
    $(id).addEventListener("input", syncSamplingPresets);
    $(id).addEventListener("change", syncSamplingPresets);
  }
  syncSamplingPresets();
}

function setStartFromModel(oneStepDown = false) {
  if (!bounds || busy || loadingModel || $("height").disabled) return;
  const step = oneStepDown ? $("z-step").valueAsNumber : 0;
  const height = bounds.max[2] - step;
  if (!Number.isFinite(height) || (oneStepDown && !(step > 0))) {
    showError("Enter a positive, finite step down before using Top − step.");
    return;
  }
  $("height").value = String(height);
  $("height").dispatchEvent(new Event("input", { bubbles: true }));
  $("height").dispatchEvent(new Event("change", { bubbles: true }));
  clearError();
}

// Keep step="any" for precise manual entries; HTML step=.1 would invalidate
// values such as .04. The buttons and arrow keys add/subtract exactly .1 mm.
function dimensionStepValue(input, direction) {
  const current = Number.isFinite(input.valueAsNumber)
    ? input.valueAsNumber
    : 0;
  return Number((current + direction * 0.1).toPrecision(15));
}

function canStepDimension(input, direction) {
  const next = dimensionStepValue(input, direction);
  return (
    !input.disabled &&
    !busy &&
    !loadingModel &&
    Number.isFinite(next) &&
    (input.min === "" || next >= Number(input.min)) &&
    (input.max === "" || next <= Number(input.max))
  );
}

function syncDimensionSteppers() {
  document.querySelectorAll(".dimension-steps button").forEach((button) => {
    button.disabled = !canStepDimension(
      $(button.dataset.input),
      Number(button.dataset.direction),
    );
  });
}

function initDimensionInputs() {
  for (const unit of document.querySelectorAll(".input-unit")) {
    if (unit.querySelector("span").textContent !== "mm") continue;
    const input = unit.querySelector("input"),
      label = document.querySelector(`label[for="${input.id}"]`).textContent;
    unit.classList.add("dimension-input");
    const buttons = document.createElement("div");
    buttons.className = "dimension-steps";
    const step = (direction) => {
      if (!canStepDimension(input, direction)) return;
      input.value = String(dimensionStepValue(input, direction));
      input.dispatchEvent(new Event("input", { bubbles: true }));
      input.dispatchEvent(new Event("change", { bubbles: true }));
    };
    for (const direction of [1, -1]) {
      const button = document.createElement("button");
      button.type = "button";
      button.textContent = direction === 1 ? "▲" : "▼";
      button.title = `${direction === 1 ? "Increase" : "Decrease"} ${label.toLowerCase()} by 0.1 mm`;
      button.setAttribute("aria-label", button.title);
      button.setAttribute("aria-controls", input.id);
      button.dataset.input = input.id;
      button.dataset.direction = direction;
      button.addEventListener("click", () => step(direction));
      buttons.append(button);
    }
    unit.append(buttons);
    input.addEventListener("input", syncDimensionSteppers);
    input.addEventListener("keydown", (event) => {
      if (event.key === "ArrowUp" || event.key === "ArrowDown") {
        event.preventDefault();
        step(event.key === "ArrowUp" ? 1 : -1);
      }
    });
  }
  syncDimensionSteppers();
}

function setBusy(value) {
  busy = value;
  $("settings").setAttribute("aria-busy", String(value));
  $("settings")
    .querySelectorAll("select,input[type=number]")
    .forEach((input) => {
      input.disabled = value || loadingModel;
    });
  $("upload").disabled = value || loadingModel;
  $("stl-file").disabled = value || loadingModel;
  for (const id of ["download", "export-gcode"])
    $(id).disabled = value || loadingModel || !lastResult?.stats.count;
  conditionalFields();
  $("threads").disabled = value || loadingModel || availableThreads === 1;
  $("run-label").textContent = value
    ? "Cancel calculation"
    : "Generate toolpath";
  $("run").disabled = loadingModel || (!ready && !value);
  $("loading-overlay").hidden = !value && !loadingModel;
  $("viewport").setAttribute("aria-busy", String(value || loadingModel));
  $("loading-title").textContent = loadingModel
    ? "Loading model"
    : "Generating toolpath";
  $("loading-cancel").hidden = loadingModel;
  $("download").disabled = value || loadingModel || !lastResult?.stats.count;
  $("play").disabled = value || loadingModel || !lastResult?.stats.count;
  $("scrub").disabled = value || loadingModel || !lastResult?.stats.count;
  clearInterval(runTimer);
  if (value || loadingModel) {
    runStarted = performance.now();
    $("loading-progress").removeAttribute("value");
    $("loading-elapsed").textContent = "0.0 s elapsed";
    runTimer = setInterval(() => {
      const elapsed = ((performance.now() - runStarted) / 1000).toFixed(1);
      $("loading-elapsed").textContent = `${elapsed} s elapsed`;
      if (busy) {
        $("stat-time").textContent = elapsed;
        $("time-unit").textContent = "s";
      }
    }, 100);
  }
}

function params() {
  const p = {
    algorithm: $("algorithm").value,
    cutter: $("cutter").value,
    diameter: number("diameter"),
    length: number("length"),
    radius: number("radius"),
    angle: number("angle"),
    sampling: number("sampling"),
    minSampling: number("min-sampling"),
    stepover: number("stepover"),
    floor: number("floor"),
    height: number("height"),
    layers: number("layers"),
    zStep: number("z-step"),
    threads: number("threads"),
    pattern: $("pattern").value,
    scanDirection: $("scan-direction").value,
    circleDirection: $("circle-direction").value,
    centerX: number("center-x"),
    centerY: number("center-y"),
    innerRadius: number("inner-radius"),
    outerRadius: number("outer-radius"),
    radialOrder: $("radial-order").value,
    startAngle: number("start-angle"),
    spiralTolerance: number("spiral-tolerance"),
    weave: $("weave").value,
    pathFilter: $("path-filter").value,
    filterTolerance: number("filter-tolerance"),
  };
  if (p.algorithm.includes("Path") && ["circles", "spiral"].includes(p.pattern))
    CAM.validateCircularSettings(p);
  if (
    p.pathFilter === "line" &&
    (!Number.isFinite(p.filterTolerance) || p.filterTolerance <= 0)
  )
    throw new Error("Filter tolerance must be finite and positive.");
  if (p.cutter === "BullCutter" && p.radius >= p.diameter / 2)
    throw new Error(
      "The corner radius must be smaller than half the cutter diameter.",
    );
  if (p.algorithm.startsWith("Adaptive") && p.minSampling > p.sampling)
    throw new Error("Minimum spacing must be no larger than sample spacing.");
  if (
    p.cutter === "ConeCutter" &&
    p.length < p.diameter / 2 / Math.tan((p.angle * Math.PI) / 360)
  )
    throw new Error("Increase the cutter length to fit the conical tip.");
  if (!p.algorithm.includes("Path")) {
    if (!Number.isSafeInteger(p.layers) || p.layers < 1)
      throw new Error(
        "Z levels must be a positive whole number within the browser’s integer range.",
      );
    if (
      !Number.isFinite(p.height) ||
      !Number.isFinite(p.zStep) ||
      p.zStep <= 0 ||
      !Number.isFinite(p.height - (p.layers - 1) * p.zStep)
    )
      throw new Error(
        "Start Z, step down, and the final Z level must be finite, with a positive step down.",
      );
    const lowestZ = p.height - (p.layers - 1) * p.zStep;
    const reach = Math.max(0, bounds.max[2] - lowestZ);
    if (p.length < reach)
      throw new Error(
        `For these waterline levels, increase cutter length to at least ${(Math.ceil(reach * 10) / 10).toFixed(1)} mm. A shorter cutter can produce false interior contours because it does not reach the model’s top from the lowest Z level.`,
      );
  }
  return p;
}

async function startEngine() {
  const generation = ++engineGeneration;
  ready = false;
  $("run").disabled = true;
  $("run-label").textContent = "Preparing model…";
  status("Preparing the CAM engine…", true);
  worker = new Worker(workerURL);
  worker.onerror = (event) => {
    if (generation !== engineGeneration) return;
    ready = false;
    loadingModel = false;
    setBusy(false);
    showError(
      event.message ||
        "The calculation worker stopped unexpectedly. Reload to restart.",
    );
    status("Engine error");
  };
  worker.onmessage = ({ data }) => {
    if (generation !== engineGeneration) return;
    if (data.type === "ready") {
      ready = true;
      availableThreads = data.maxThreads;
      const previous = Number($("threads").value);
      $("threads").replaceChildren(
        ...Array.from(
          { length: availableThreads },
          (_, i) => new Option(`${i + 1} thread${i ? "s" : ""}`, String(i + 1)),
        ),
      );
      $("threads").value = String(
        generation === 1
          ? availableThreads
          : Math.min(previous, availableThreads),
      );
      $("engine-badge").innerHTML =
        `<span class="dot"></span>${availableThreads > 1 ? "Multithreaded WASM" : "Single-thread WASM"}`;
      $("thread-help").textContent =
        availableThreads > 1
          ? "Up to four threads. The preview stays responsive while calculations run."
          : "Running in a background worker. Use the included local server to enable more threads.";
      loadingModel = false;
      setBusy(false);
      status(
        lastResult
          ? "Ready for another pass"
          : "Model ready. Choose a strategy and generate a path.",
      );
      if (!lastResult && generation === 1) $("settings").requestSubmit();
    } else if (data.type === "progress") {
      status(data.text, true);
      if (data.total) {
        $("loading-progress").max = data.total;
        $("loading-progress").value = data.completed;
      }
    } else if (data.type === "result") {
      lastResult = {
        ...data,
        model: modelName,
        params: { ...activeParams },
        bounds: { min: [...bounds.min], max: [...bounds.max] },
      };
      setBusy(false);
      displayResult();
    } else if (data.type === "error") {
      if (loadingModel) ready = false;
      loadingModel = false;
      setBusy(false);
      showError(data.message);
      status("Calculation failed. Adjust the settings and try again.");
    }
  };
  const copy = geometry.getAttribute("position").array.slice().buffer;
  worker.postMessage({ type: "init", url: libraryURL, positions: copy }, [
    copy,
  ]);
}

function cancel() {
  worker?.terminate();
  playing = false;
  $("play").textContent = "▶";
  setBusy(false);
  status("Calculation cancelled");
  if (lastResult) updateStats(lastResult.stats);
  else {
    $("stat-time").textContent = "—";
    $("time-unit").textContent = "ms";
  }
  startEngine();
}

function generate(event) {
  event?.preventDefault();
  if (busy) {
    cancel();
    return;
  }
  if (!ready) return;
  try {
    clearError();
    activeParams = params();
    playing = false;
    $("play").textContent = "▶";
    setBusy(true);
    status("Building the spatial index…", true);
    worker.postMessage({ type: "run", params: activeParams, bounds });
  } catch (error) {
    showError(error.message);
  }
}

function clearGroup(group) {
  for (const child of [...group.children]) {
    child.geometry?.dispose();
    child.material?.dispose();
    group.remove(child);
  }
}

const cutterPresets = {
  ball: {
    cutter: "BallCutter",
    diameter: 0.8,
    length: 10,
    radius: 0.15,
    angle: 60,
  },
  flat: {
    cutter: "CylCutter",
    diameter: 1,
    length: 10,
    radius: 0.2,
    angle: 60,
  },
  bull: {
    cutter: "BullCutter",
    diameter: 1,
    length: 10,
    radius: 0.2,
    angle: 60,
  },
  cone: {
    cutter: "ConeCutter",
    diameter: 1,
    length: 10,
    radius: 0.2,
    angle: 60,
  },
};
function applyPreset() {
  const preset = cutterPresets[$("cutter-preset").value];
  if (!preset) return;
  for (const [id, value] of Object.entries(preset)) $(id).value = String(value);
  if (bounds) {
    const bottom = Math.min(
      bounds.min[2],
      Number.isFinite(number("floor")) ? number("floor") : bounds.min[2],
    );
    $("length").value = String(
      Math.ceil(Math.max(preset.length, bounds.max[2] - bottom) * 10) / 10,
    );
  }
  conditionalFields();
  clearError();
}

function parseModel(buffer) {
  if (!buffer.byteLength) throw new Error("This STL file is empty.");
  const g = new STLLoader().parse(buffer);
  try {
    const positions = g.getAttribute("position");
    if (!positions || !positions.count || positions.count % 3)
      throw new Error("No complete triangles were found in this STL.");
    const a = new THREE.Vector3(),
      b = new THREE.Vector3(),
      c = new THREE.Vector3();
    for (let i = 0; i < positions.count; i += 3) {
      a.fromBufferAttribute(positions, i);
      b.fromBufferAttribute(positions, i + 1);
      c.fromBufferAttribute(positions, i + 2);
      if (![a.x, a.y, a.z, b.x, b.y, b.z, c.x, c.y, c.z].every(Number.isFinite))
        throw new Error("The STL contains invalid vertex coordinates.");
      if (b.sub(a).cross(c.sub(a)).lengthSq() === 0)
        throw new Error(
          `Triangle ${i / 3 + 1} has zero area. Repair the STL and try again.`,
        );
    }
    g.computeVertexNormals();
    g.computeBoundingBox();
    return g;
  } catch (error) {
    g.dispose();
    throw error;
  }
}

function fitCircularPaths() {
  const diameter = number("diameter");
  if (!bounds || !Number.isFinite(diameter) || diameter <= 0) {
    showError(
      "Set a finite, positive cutter diameter before fitting the path.",
    );
    return;
  }
  const fitted = CAM.fitCircularBounds(bounds, diameter);
  for (const [id, key] of [
    ["center-x", "centerX"],
    ["center-y", "centerY"],
    ["inner-radius", "innerRadius"],
    ["outer-radius", "outerRadius"],
  ])
    $(id).value = String(fitted[key]);
  clearError();
  syncDimensionSteppers();
}

function updateModelView() {
  bounds = {
    min: geometry.boundingBox.min.toArray(),
    max: geometry.boundingBox.max.toArray(),
  };
  const sizes = bounds.max.map((v, i) => v - bounds.min[i]),
    extent = Math.max(...sizes);
  $("height-model-top").title = `Set Start Z to ${bounds.max[2]} mm`;
  $("height-top-step").title =
    "Set Start Z to the model’s highest Z minus the current step down";
  $("model-info").textContent =
    `${fmt.format(geometry.getAttribute("position").count / 3)} triangles · ${sizes.map((v) => v.toFixed(1)).join(" × ")} mm`;
  $("model-title").textContent =
    currentSource === "tux" ? "Tux" : modelName.replace(/\.stl$/i, "");
  $("model-name").textContent = `◇ ${modelName}`;
  $("model-name").title = modelName;
  if (grid) {
    scene.remove(grid);
    grid.geometry.dispose();
    grid.material.dispose();
  }
  grid = new THREE.GridHelper(extent * 2.5, 60, 0xb5c2ae, 0xd6decf);
  grid.rotation.x = Math.PI / 2;
  grid.position.set(
    (bounds.min[0] + bounds.max[0]) / 2,
    (bounds.min[1] + bounds.max[1]) / 2,
    bounds.min[2] - extent / 300,
  );
  grid.material.transparent = true;
  grid.material.opacity = 0.65;
  scene.add(grid);
  camera.near = extent / 10000;
  camera.far = extent * 100;
  camera.updateProjectionMatrix();
  controls.minDistance = extent / 100;
  controls.maxDistance = extent * 20;
  cameraView();
}

function clearResult() {
  lastResult = undefined;
  invalidateGcode();
  playing = false;
  playback = 0;
  $("play").textContent = "▶";
  $("scrub").value = "0";
  clearGroup(pathGroup);
  if (cutterMesh) {
    scene.remove(cutterMesh);
    cutterMesh.geometry.dispose();
    cutterMesh.material.dispose();
    cutterMesh = undefined;
  }
  for (const id of ["stat-time", "stat-points", "stat-length", "stat-paths"])
    $(id).textContent = "—";
  $("time-unit").textContent = "ms";
  $("detail-stats").textContent = "OpenCAMLib · Three.js";
  updateCutter();
}

async function selectModel(buffer, name, source) {
  if (busy || loadingModel) return;
  loadingModel = true;
  setBusy(false);
  clearError();
  status("Reading and validating STL triangles…", true);
  try {
    await new Promise(requestAnimationFrame);
    const next = parseModel(buffer);
    worker?.terminate();
    ++engineGeneration;
    ready = false;
    model.geometry = next;
    geometry.dispose();
    geometry = next;
    modelName = name;
    currentSource = source;
    $("model-source").value = source;
    if (source === "upload") {
      uploadedSTL = { buffer, name };
      $("uploaded-option").disabled = false;
      $("uploaded-option").textContent = name;
    }
    clearResult();
    updateModelView();
    const extent = Math.max(...bounds.max.map((v, i) => v - bounds.min[i]));
    const defaults =
      source === "tux"
        ? { stepover: 0.4, floor: -3, height: 2, "z-step": 0.5 }
        : {
            stepover: extent / 30,
            floor: bounds.min[2] - extent / 50,
            height: bounds.max[2],
            "z-step": Math.max(
              (bounds.max[2] - bounds.min[2]) / 10,
              extent / 100,
            ),
          };
    // Preserve the exact top; rounding upward can put a waterline above the STL.
    for (const [id, value] of Object.entries(defaults))
      $(id).value = String(
        id === "height" ? value : Number(value.toPrecision(7)),
      );
    $("layers").value = "1";
    // Refit preset lengths to this geometry; preserve explicitly customized tools.
    applyPreset();
    fitCircularPaths();
    await startEngine();
  } catch (error) {
    loadingModel = false;
    setBusy(false);
    $("model-source").value = currentSource;
    showError(`Could not load STL: ${error.message}`);
    status("Model unchanged. Choose another STL.");
  }
}

function updateStats(s) {
  $("stat-time").textContent =
    s.computeMs < 1000
      ? s.computeMs.toFixed(0)
      : (s.computeMs / 1000).toFixed(2);
  $("time-unit").textContent = s.computeMs < 1000 ? "ms" : "s";
  $("stat-points").textContent = fmt.format(s.count);
  $("stat-length").textContent = s.length.toFixed(1);
  $("stat-paths").textContent = fmt.format(s.paths);
  $("paths-label").textContent = lastResult.params.algorithm.includes("Path")
    ? "Passes"
    : "Contours";
  $("detail-stats").textContent =
    `${(s.totalMs / 1000).toFixed(2)} s incl. setup · up to ${s.threads} compute thread${s.threads > 1 ? "s" : ""}${lastResult.params.pathFilter === "line" ? ` · ${fmt.format(s.rawCount - s.count)} points filtered` : ""}`;
}

function displayResult() {
  const { coordinates: positions, offsets, stats: s } = lastResult;
  clearGroup(pathGroup);
  for (let i = 0; i < offsets.length - 1; i++) {
    const points = new Float32Array(
      positions.subarray(offsets[i] * 3, offsets[i + 1] * 3),
    );
    const g = new THREE.BufferGeometry();
    g.setAttribute("position", new THREE.BufferAttribute(points, 3));
    const line = new THREE.Line(
      g,
      new THREE.LineBasicMaterial({ color: 0xe86b32, depthTest: true }),
    );
    line.renderOrder = 2;
    pathGroup.add(line);
  }
  updateStats(s);
  $("download").disabled = !s.count;
  $("export-gcode").disabled = !s.count;
  invalidateGcode();
  $("play").disabled = !s.count;
  $("scrub").disabled = !s.count;
  playback = 0;
  $("scrub").value = "0";
  makeCutter();
  updateCutter();
  status(
    s.count
      ? `Toolpath ready · ${fmt.format(s.count)} points`
      : "No contours at these heights. Try a different start Z.",
  );
}

function makeCutter() {
  if (cutterMesh) {
    scene.remove(cutterMesh);
    cutterMesh.geometry.dispose();
    cutterMesh.material.dispose();
  }
  const p = lastResult.params,
    r = p.diameter / 2;
  const profile = [new THREE.Vector2(0, 0)];
  if (p.cutter === "BallCutter" || p.cutter === "BullCutter") {
    const fillet = p.cutter === "BallCutter" ? r : p.radius;
    const angle = Math.acos(1 - Math.min(p.length, fillet) / fillet);
    for (let i = 0; i <= 20; i++) {
      const a = (i * angle) / 20;
      profile.push(
        new THREE.Vector2(
          r - fillet + fillet * Math.sin(a),
          fillet * (1 - Math.cos(a)),
        ),
      );
    }
  } else if (p.cutter === "ConeCutter")
    profile.push(new THREE.Vector2(r, r / Math.tan((p.angle * Math.PI) / 360)));
  else profile.push(new THREE.Vector2(r, 0));
  profile.push(
    new THREE.Vector2(profile[profile.length - 1].x, p.length),
    new THREE.Vector2(0, p.length),
  );
  cutterMesh = new THREE.Mesh(
    new THREE.LatheGeometry(profile, 32),
    new THREE.MeshStandardMaterial({
      color: 0x2a8175,
      metalness: 0.45,
      roughness: 0.28,
      transparent: true,
      opacity: 0.72,
    }),
  );
  cutterMesh.rotation.x = Math.PI / 2;
  scene.add(cutterMesh);
}

function updateCutter() {
  $("scrub-value").textContent = `${Math.round(playback * 100)}%`;
  const count = lastResult?.stats.count || 0;
  $("tip-readout").dataset.empty = String(!count);
  if (!count) {
    for (const axis of ["x", "y", "z"]) {
      $(`tip-${axis}`).textContent = "—";
      $(`tip-${axis}`).removeAttribute("title");
    }
    $("tip-context").textContent =
      "Generate a toolpath to inspect its coordinates.";
    if (cutterMesh) cutterMesh.visible = false;
    return;
  }
  const sample = Math.min(count - 1, Math.floor(playback * (count - 1))),
    i = sample * 3;
  const { coordinates, offsets, params: p, stats: s } = lastResult;
  for (const [axis, j] of [
    ["x", 0],
    ["y", 1],
    ["z", 2],
  ]) {
    const value = coordinates[i + j];
    $(`tip-${axis}`).textContent =
      Math.abs(value) < 0.0005 ? "0.000" : value.toFixed(3);
    $(`tip-${axis}`).title = `${value} mm`;
  }
  let low = 0,
    high = offsets.length - 1;
  while (low + 1 < high) {
    const mid = (low + high) >>> 1;
    if (offsets[mid] <= sample) low = mid;
    else high = mid;
  }
  const level = s.levels.findIndex((entry) => entry.z === coordinates[i + 2]);
  $("tip-context").textContent =
    `${p.algorithm.includes("Path") ? "Pass" : "Contour"} ${low + 1}/${s.paths}${level >= 0 ? ` · Z level ${level + 1}/${s.levels.length}` : ""} · Point ${fmt.format(sample + 1)}/${fmt.format(count)}`;
  if (cutterMesh) {
    cutterMesh.position.fromArray(coordinates, i);
    cutterMesh.visible = $("show-cutter").checked;
  }
}

function cameraView(top = false) {
  if (!camera || !bounds) return;
  const box = new THREE.Box3(
    new THREE.Vector3(...bounds.min),
    new THREE.Vector3(...bounds.max),
  );
  if (cutterMesh?.visible) box.expandByObject(cutterMesh);
  const center = box.getCenter(new THREE.Vector3()),
    radius = box.getSize(new THREE.Vector3()).length() / 2;
  const halfFov = Math.atan(
    Math.tan((camera.fov * Math.PI) / 360) * Math.min(1, camera.aspect),
  );
  const distance = (radius / Math.sin(halfFov)) * 1.12;
  camera.near = radius / 5000;
  camera.far = radius * 200;
  camera.updateProjectionMatrix();
  controls.minDistance = radius / 50;
  controls.maxDistance = radius * 40;
  camera.up.set(0, 0, 1);
  const direction = top
    ? new THREE.Vector3(0, -0.0001, 1)
    : new THREE.Vector3(0.7, -1, 0.95);
  camera.position.copy(center).addScaledVector(direction.normalize(), distance);
  controls.target.copy(center);
  camera.lookAt(center);
  controls.update();
}

async function initView() {
  [THREE] = await Promise.all([import("three")]);
  const [{ OrbitControls }, loader] = await Promise.all([
    import("three/addons/controls/OrbitControls.js"),
    import("three/addons/loaders/STLLoader.js"),
  ]);
  STLLoader = loader.STLLoader;
  geometry = parseModel(stl);
  scene = new THREE.Scene();
  renderer = new THREE.WebGLRenderer({
    canvas: $("scene"),
    antialias: true,
    alpha: true,
  });
  renderer.setPixelRatio(Math.min(devicePixelRatio, 2));
  renderer.setClearColor(0, 0);
  renderer.outputColorSpace = THREE.SRGBColorSpace;
  renderer.toneMapping = THREE.ACESFilmicToneMapping;
  renderer.toneMappingExposure = 1.3;
  camera = new THREE.PerspectiveCamera(35, 1, 0.01, 500);
  camera.up.set(0, 0, 1);
  controls = new OrbitControls(camera, renderer.domElement);
  controls.enableDamping = true;
  controls.dampingFactor = 0.08;
  controls.minDistance = 2;
  controls.maxDistance = 100;
  scene.add(new THREE.AmbientLight(0xffffff, 1.2));
  const key = new THREE.DirectionalLight(0xfff6e4, 3);
  key.position.set(-8, -12, 22);
  scene.add(key);
  const fill = new THREE.DirectionalLight(0xd9ebff, 2);
  fill.position.set(15, 8, 10);
  scene.add(fill);
  model = new THREE.Mesh(
    geometry,
    new THREE.MeshStandardMaterial({
      color: 0x789283,
      metalness: 0.22,
      roughness: 0.48,
      side: THREE.DoubleSide,
      polygonOffset: true,
      polygonOffsetFactor: 1,
      polygonOffsetUnits: 1,
    }),
  );
  scene.add(model);
  updateModelView();
  fitCircularPaths();
  pathGroup = new THREE.Group();
  scene.add(pathGroup);
  const resize = () => {
    const rect = $("viewport").getBoundingClientRect();
    renderer.setSize(rect.width, rect.height, false);
    camera.aspect = rect.width / rect.height;
    camera.updateProjectionMatrix();
  };
  new ResizeObserver(resize).observe($("viewport"));
  resize();
  cameraView();
  renderer.setAnimationLoop((time) => {
    const dt = Math.min(0.1, (time - lastFrame) / 1000);
    lastFrame = time;
    if (playing && lastResult) {
      playback = Math.min(1, playback + dt / 30);
      $("scrub").value = String(Math.round(playback * 1000));
      updateCutter();
      if (playback === 1) {
        playing = false;
        $("play").textContent = "▶";
      }
    }
    controls.update();
    renderer.render(scene, camera);
  });
}

$("settings").addEventListener("submit", generate);
$("algorithm").addEventListener("change", conditionalFields);
$("pattern").addEventListener("change", conditionalFields);
$("fit-circles").addEventListener("click", fitCircularPaths);
$("path-filter").addEventListener("change", conditionalFields);
$("height-model-top").addEventListener("click", () => setStartFromModel());
$("height-top-step").addEventListener("click", () => setStartFromModel(true));
$("cutter-preset").addEventListener("change", applyPreset);
for (const id of ["cutter", "diameter", "length", "radius", "angle"])
  $(id).addEventListener("input", () => {
    $("cutter-preset").value = "custom";
    conditionalFields();
  });
$("cutter").addEventListener("change", () => {
  $("cutter-preset").value = "custom";
  conditionalFields();
});
$("loading-cancel").addEventListener("click", cancel);
$("upload").addEventListener("click", () => {
  $("stl-file").value = "";
  $("stl-file").click();
});
$("stl-file").addEventListener("change", async () => {
  const file = $("stl-file").files[0];
  if (!file || busy || loadingModel) return;
  try {
    if (!file.name.toLowerCase().endsWith(".stl"))
      throw new Error("Choose a file with the .stl extension.");
    if (file.size > 100 * 1024 * 1024)
      throw new Error("This browser app supports STL files up to 100 MB.");
    await selectModel(await file.arrayBuffer(), file.name, "upload");
  } catch (error) {
    showError(error.message);
  }
});
$("model-source").addEventListener("change", () => {
  if ($("model-source").value === "tux")
    selectModel(stl, "gnu_tux_mod.stl", "tux");
  else if (uploadedSTL)
    selectModel(uploadedSTL.buffer, uploadedSTL.name, "upload");
});
$("show-model").addEventListener("change", () => {
  if (model) model.visible = $("show-model").checked;
});
$("show-path").addEventListener("change", () => {
  if (pathGroup) pathGroup.visible = $("show-path").checked;
});
$("wireframe").addEventListener("change", () => {
  if (model) model.material.wireframe = $("wireframe").checked;
});
$("show-cutter").addEventListener("change", updateCutter);
$("top-view").addEventListener("click", () => cameraView(true));
$("iso-view").addEventListener("click", () => cameraView());
$("fit-view").addEventListener("click", () => cameraView());
$("scrub").addEventListener("input", () => {
  playing = false;
  $("play").textContent = "▶";
  playback = number("scrub") / 1000;
  $("show-cutter").checked = true;
  updateCutter();
});
$("play").addEventListener("click", () => {
  if (playback >= 1) playback = 0;
  playing = !playing;
  $("play").textContent = playing ? "Ⅱ" : "▶";
  $("show-cutter").checked = true;
  updateCutter();
});
$("download").addEventListener("click", () => {
  const { coordinates, offsets } = lastResult;
  const paths = Array.from({ length: offsets.length - 1 }, (_, i) =>
    Array.from({ length: offsets[i + 1] - offsets[i] }, (_, j) =>
      Array.from(
        coordinates.subarray((offsets[i] + j) * 3, (offsets[i] + j + 1) * 3),
      ),
    ),
  );
  const data = {
    model: lastResult.model,
    units: "mm",
    parameters: lastResult.params,
    statistics: lastResult.stats,
    paths,
  };
  downloadFile(JSON.stringify(data, null, 2), "application/json", "json");
});

function downloadFile(text, type, extension) {
  const url = URL.createObjectURL(new Blob([text], { type }));
  const a = document.createElement("a");
  a.href = url;
  a.download = `${lastResult.model.replace(/\.stl$/i, "").replace(/[^a-z0-9_-]/gi, "_")}-${lastResult.params.algorithm}.${extension}`;
  a.click();
  setTimeout(() => URL.revokeObjectURL(url), 1000);
}

let gcodeProgram, gcodeSource;
function invalidateGcode() {
  gcodeProgram = undefined;
  $("gcode-output").hidden = true;
  $("gcode-download").disabled = true;
  $("gcode-error").hidden = true;
  $("gcode-preview").value = "";
}
$("export-gcode").addEventListener("click", () => {
  if (!lastResult?.stats.count || busy || loadingModel) return;
  if (gcodeSource !== lastResult) {
    let top = lastResult.bounds.max[2];
    for (let i = 2; i < lastResult.coordinates.length; i += 3)
      top = Math.max(top, lastResult.coordinates[i]);
    $("gcode-retract").value = String(Math.ceil((top + 5) * 10) / 10);
    gcodeSource = lastResult;
    invalidateGcode();
  }
  $("gcode-summary").textContent =
    `${lastResult.model} · ${lastResult.params.algorithm} · ${fmt.format(lastResult.stats.paths)} separate paths · ${fmt.format(lastResult.stats.count)} points. Exports the last generated toolpath.`;
  syncDimensionSteppers();
  $("gcode-dialog").showModal();
});
$("gcode-close").addEventListener("click", () => $("gcode-dialog").close());
$("gcode-form").addEventListener("input", invalidateGcode);
$("gcode-form").addEventListener("change", () => {
  invalidateGcode();
  for (const id of ["gcode-rpm", "gcode-dwell"])
    $(id).disabled = $("gcode-spindle").value === "off";
});
$("gcode-form").addEventListener("submit", (event) => {
  event.preventDefault();
  invalidateGcode();
  try {
    const manual = $("gcode-spindle").value === "off";
    gcodeProgram = CAM.writeGcode(lastResult, {
      retract: $("gcode-retract").valueAsNumber,
      workOffset: $("gcode-offset").value,
      feed: $("gcode-feed").valueAsNumber,
      plunge: $("gcode-plunge").valueAsNumber,
      spindle: $("gcode-spindle").value,
      rpm: manual ? 0 : $("gcode-rpm").valueAsNumber,
      dwell: manual ? 0 : $("gcode-dwell").valueAsNumber,
      coolant: $("gcode-coolant").value,
      decimals: number("gcode-decimals"),
    });
    $("gcode-preview").value = gcodeProgram.text;
    $("gcode-info").textContent =
      `${fmt.format(gcodeProgram.lines)} lines · ${gcodeProgram.paths} paths · ${(new Blob([gcodeProgram.text]).size / 1024).toFixed(1)} KiB`;
    $("gcode-output").hidden = false;
    $("gcode-download").disabled = false;
  } catch (error) {
    $("gcode-error").textContent = error.message;
    $("gcode-error").hidden = false;
  }
});
$("gcode-download").addEventListener("click", () => {
  if (gcodeProgram) downloadFile(gcodeProgram.text, "text/plain", "nc");
});
window.addEventListener("beforeunload", () => {
  worker?.terminate();
  URL.revokeObjectURL(workerURL);
  URL.revokeObjectURL(libraryURL);
});

try {
  initDimensionInputs();
  initSamplingPresets();
  $("upload").disabled = true;
  $("model-source").disabled = true;
  if (!("DecompressionStream" in window))
    throw new Error(
      "This app needs a current browser with gzip decompression support.",
    );
  const threaded =
    crossOriginIsolated && typeof SharedArrayBuffer !== "undefined";
  const [mesh, library] = await Promise.all([
    inflate("asset-stl"),
    inflate(threaded ? "asset-threaded" : "asset-serial"),
  ]);
  stl = mesh;
  libraryURL = URL.createObjectURL(
    new Blob([library], { type: "application/javascript" }),
  );
  workerURL = URL.createObjectURL(
    new Blob(
      [
        library,
        "\n",
        $("cam-source").textContent,
        "\n",
        $("worker-source").textContent,
      ],
      {
        type: "application/javascript",
      },
    ),
  );
  await initView();
  conditionalFields();
  await startEngine();
} catch (error) {
  showError(
    `${error.message}. Check that your browser can reach cdn.jsdelivr.net, then reload.`,
  );
  status("Unable to load the app");
  $("engine-badge").textContent = "Loading failed";
}
