// Browser regression for the single-file app. Run from the repository root.
// Fixtures are generated locally; no private STL files are required.
// Set CHROME_BIN to override the Chrome executable.
import assert from "node:assert/strict";
import {
  readFile,
  writeFile,
  mkdtemp,
  rm,
  readdir,
  mkdir,
} from "node:fs/promises";
import { spawn } from "node:child_process";
import { createServer } from "node:http";
import { tmpdir } from "node:os";
import { join, resolve } from "node:path";
import { pathToFileURL } from "node:url";
import { setTimeout as delay } from "node:timers/promises";

const html = await readFile("examples/web/index.html");
const server = createServer((req, res) => {
  if (req.url === "/favicon.ico") {
    res.writeHead(204);
    res.end();
    return;
  }
  if (req.url !== "/" && req.url !== "/opencamlib/") {
    res.writeHead(404);
    res.end();
    return;
  }
  if (req.url === "/") {
    res.setHeader("Cross-Origin-Opener-Policy", "same-origin");
    res.setHeader("Cross-Origin-Embedder-Policy", "require-corp");
  }
  res.setHeader("Content-Type", "text/html; charset=utf-8");
  res.end(html);
});
await new Promise((r) => server.listen(0, "127.0.0.1", r));
const base = `http://127.0.0.1:${server.address().port}`;
const artifacts = resolve(process.env.OCL_WEB_ARTIFACTS || "build/web");
await mkdir(artifacts, { recursive: true });
const profile = await mkdtemp(join(tmpdir(), "ocl-web-test-"));
const downloads = await mkdtemp(join(profile, "downloads-"));
const screenshots = [];
const chromeBinary =
  process.env.CHROME_BIN ||
  (process.platform === "darwin"
    ? "/Applications/Google Chrome.app/Contents/MacOS/Google Chrome"
    : "google-chrome");
const chrome = spawn(
  chromeBinary,
  [
    "--headless",
    "--disable-background-networking",
    "--disable-component-update",
    "--no-first-run",
    "--no-default-browser-check",
    "--enable-unsafe-swiftshader",
    "--use-angle=swiftshader",
    "--remote-debugging-port=0",
    `--user-data-dir=${profile}`,
    "about:blank",
  ],
  { stdio: ["ignore", "ignore", "pipe"] },
);
let launchError;
chrome.on("error", (error) => {
  launchError = error;
});
let logs = "";
chrome.stderr.on("data", (d) => {
  logs = (logs + d).slice(-8000);
});
let socket;
try {
  let port;
  for (let i = 0; i < 100; i++) {
    if (launchError) throw launchError;
    try {
      port = Number(
        (await readFile(join(profile, "DevToolsActivePort"), "utf8")).split(
          "\n",
        )[0],
      );
      break;
    } catch {}
    await delay(100);
  }
  if (!port) throw Error("Chrome did not start: " + logs);
  const targets = await fetch(`http://127.0.0.1:${port}/json/list`).then((r) =>
    r.json(),
  );
  socket = new WebSocket(
    targets.find((t) => t.type === "page").webSocketDebuggerUrl,
  );
  await new Promise((r, j) => {
    socket.addEventListener("open", r, { once: true });
    socket.addEventListener("error", j, { once: true });
  });
  let seq = 0;
  const pending = new Map(),
    exceptions = [],
    requests = [];
  socket.addEventListener("message", (e) => {
    const m = JSON.parse(e.data);
    if (m.id) {
      const p = pending.get(m.id);
      if (!p) return;
      pending.delete(m.id);
      m.error ? p.reject(Error(JSON.stringify(m.error))) : p.resolve(m.result);
    } else if (m.method === "Runtime.exceptionThrown")
      exceptions.push(m.params.exceptionDetails);
    else if (m.method === "Network.requestWillBeSent")
      requests.push(m.params.request.url);
  });
  function cdp(method, params = {}) {
    return new Promise((resolve, reject) => {
      const id = ++seq;
      const timeout = setTimeout(() => {
        pending.delete(id);
        reject(Error(`Chrome did not respond to ${method} within 30 seconds`));
      }, 30000);
      pending.set(id, {
        resolve: (value) => {
          clearTimeout(timeout);
          resolve(value);
        },
        reject: (error) => {
          clearTimeout(timeout);
          reject(error);
        },
      });
      socket.send(JSON.stringify({ id, method, params }));
    });
  }
  async function evaluate(expression) {
    const r = await cdp("Runtime.evaluate", {
      expression,
      returnByValue: true,
      awaitPromise: true,
    });
    if (r.exceptionDetails) throw Error(JSON.stringify(r.exceptionDetails));
    return r.result.value;
  }
  async function waitFor(expression, timeout = 180000) {
    const end = Date.now() + timeout;
    while (Date.now() < end) {
      if (await evaluate(expression)) return;
      const err = await evaluate(
        "document.getElementById('error') && !document.getElementById('error').hidden ? document.getElementById('error').textContent : ''",
      );
      if (err) throw Error(err);
      await delay(100);
    }
    throw Error(
      "Timed out: " +
        expression +
        "; " +
        (await evaluate("document.body.innerText")),
    );
  }
  const idle =
    "document.getElementById('status')?.textContent.startsWith('Toolpath ready') && document.getElementById('settings').getAttribute('aria-busy')==='false'";
  async function screenshot(name) {
    const r = await cdp("Page.captureScreenshot", { format: "png" });
    const path = join(artifacts, name);
    await writeFile(path, Buffer.from(r.data, "base64"));
    screenshots.push(path);
  }
  async function run(algorithm, cutter, values = {}) {
    await evaluate(
      `(()=>{for(const [id,value] of Object.entries(${JSON.stringify({ algorithm, cutter, ...values })})){const e=document.getElementById(id);e.value=value;e.dispatchEvent(new Event('change',{bubbles:true}));}document.getElementById('settings').requestSubmit();return true})()`,
    );
    await waitFor(idle);
    const result = await evaluate(
      `Object.fromEntries(['stat-points','stat-time','stat-paths','stat-length','detail-stats'].map(id=>[id,document.getElementById(id).textContent]))`,
    );
    assert(Number(result["stat-points"].replaceAll(",", "")) > 0);
    return result;
  }
  await cdp("Page.enable");
  await cdp("Runtime.enable");
  await cdp("Network.enable");
  await cdp("Browser.setDownloadBehavior", {
    behavior: "allow",
    downloadPath: downloads,
  });
  async function exportPoints() {
    for (const file of await readdir(downloads))
      await rm(join(downloads, file));
    const before = new Set();
    await evaluate("document.getElementById('download').click()");
    for (let i = 0; i < 100; i++) {
      for (const file of await readdir(downloads)) {
        if (!before.has(file) && file.endsWith(".json")) {
          const data = JSON.parse(
            await readFile(join(downloads, file), "utf8"),
          );
          assert.equal(data.units, "mm");
          assert.equal(
            data.paths.reduce((n, p) => n + p.length, 0),
            data.statistics.count,
          );
          assert(data.paths.flat(2).every(Number.isFinite));
          return data;
        }
      }
      await delay(50);
    }
    throw Error("JSON download did not complete");
  }
  async function exportGcode() {
    for (const file of await readdir(downloads))
      await rm(join(downloads, file));
    await evaluate("document.getElementById('gcode-download').click()");
    for (let i = 0; i < 100; i++) {
      for (const file of await readdir(downloads)) {
        if (file.endsWith(".nc"))
          return readFile(join(downloads, file), "utf8");
      }
      await delay(50);
    }
    throw Error("G-code download did not complete");
  }
  await cdp("Emulation.setDeviceMetricsOverride", {
    width: 1440,
    height: 960,
    deviceScaleFactor: 1,
    mobile: false,
  });
  await cdp("Page.navigate", { url: base + "/" });
  await waitFor(idle);
  console.log(
    "INITIAL",
    await evaluate(
      "({isolated:crossOriginIsolated,threads:document.getElementById('threads').value,points:document.getElementById('stat-points').textContent})",
    ),
  );
  await screenshot("tux-web-desktop.png");

  const samplingValues = () =>
    evaluate(
      "['sampling','min-sampling'].map(id=>Number(document.getElementById(id).value))",
    );
  assert.deepEqual(await samplingValues(), [0.05, 0.005]);
  assert.equal(
    await evaluate(
      "document.getElementById('sampling-fine').getAttribute('aria-pressed')",
    ),
    "true",
  );
  const initialPoints = await evaluate(
    "document.getElementById('stat-points').textContent",
  );
  for (const [preset, values] of [
    ["preview", [0.25, 0.05]],
    ["standard", [0.1, 0.01]],
    ["veryFine", [0.02, 0.002]],
    ["fine", [0.05, 0.005]],
  ]) {
    await evaluate(`document.getElementById('sampling-${preset}').click()`);
    assert.deepEqual(await samplingValues(), values);
    assert.equal(
      await evaluate(
        `document.getElementById('sampling-${preset}').getAttribute('aria-pressed')`,
      ),
      "true",
    );
    assert.equal(
      await evaluate(
        "document.querySelectorAll('#sampling-presets [aria-pressed=true]').length",
      ),
      1,
    );
    assert.equal(
      await evaluate("document.getElementById('settings').checkValidity()"),
      true,
    );
    assert.equal(
      await evaluate("document.getElementById('loading-overlay').hidden"),
      true,
    );
    assert.equal(
      await evaluate("document.getElementById('stat-points').textContent"),
      initialPoints,
    );
  }
  await evaluate(
    "document.getElementById('sampling').value='.037';document.getElementById('sampling').dispatchEvent(new Event('input'));document.getElementById('min-sampling').value='.003';document.getElementById('min-sampling').dispatchEvent(new Event('input'))",
  );
  assert.equal(
    await evaluate(
      "document.querySelectorAll('#sampling-presets [aria-pressed=true]').length",
    ),
    0,
  );
  assert.match(
    await evaluate("document.getElementById('sampling-help').textContent"),
    /^Custom sampling/,
  );
  await evaluate(
    "document.getElementById('algorithm').value='PathDropCutter';document.getElementById('algorithm').dispatchEvent(new Event('change'));document.getElementById('sampling-standard').click()",
  );
  assert.deepEqual(await samplingValues(), [0.1, 0.01]);
  assert.equal(
    await evaluate("document.getElementById('min-sampling').disabled"),
    true,
  );
  assert.match(
    await evaluate("document.getElementById('sampling-help').textContent"),
    /Uniform sampling uses sample spacing only/,
  );
  await evaluate(
    "document.getElementById('algorithm').value='AdaptivePathDropCutter';document.getElementById('algorithm').dispatchEvent(new Event('change'));document.getElementById('sampling-fine').click()",
  );
  assert.deepEqual(await samplingValues(), [0.05, 0.005]);
  console.log(
    "SAMPLING_PRESETS fixed fine defaults, all buttons, custom edits, uniform/adaptive modes and no automatic calculation pass",
  );

  // Observe actual Three.js objects without adding a test API to the app.
  await evaluate(`(async()=>{
  const THREE=await import('three');
  const add=THREE.Object3D.prototype.add, update=THREE.PerspectiveCamera.prototype.updateProjectionMatrix;
  THREE.Object3D.prototype.add=function(...objects){for(const object of objects)if(object.geometry?.type==='LatheGeometry')window.testCutter=object;return add.apply(this,objects)};
  THREE.PerspectiveCamera.prototype.updateProjectionMatrix=function(){window.testCamera=this;return update.call(this)};
  window.cutterBounds=()=>{const b=new THREE.Box3().setFromObject(window.testCutter);return {min:b.min.toArray(),max:b.max.toArray()}};
  window.cutterFits=()=>{
   const b=new THREE.Box3().setFromObject(window.testCutter),c=window.testCamera;c.updateMatrixWorld();
   return [b.min.x,b.max.x].every(x=>[b.min.y,b.max.y].every(y=>[b.min.z,b.max.z].every(z=>{
    const v=new THREE.Vector3(x,y,z).project(c);return [v.x,v.y,v.z].every(n=>Math.abs(n)<=1);
   })));
  };
 })()`);
  async function checkCutter(data) {
    await checkTip(data, 0);
    const box = await evaluate("window.cutterBounds()"),
      tip = data.paths[0][0],
      p = data.parameters;
    const close = (a, b) => assert(Math.abs(a - b) < 1e-5, `${a} != ${b}`);
    close(box.min[2], tip[2]);
    close(box.max[2], tip[2] + p.length);
    close(box.max[0] - box.min[0], p.diameter);
    close(box.max[1] - box.min[1], p.diameter);
    await evaluate("document.getElementById('fit-view').click()");
    assert.equal(await evaluate("window.cutterFits()"), true);
  }

  // Millimetre buttons and keyboard arrows preserve fine typed values.
  async function dimension(id, initial) {
    await evaluate(
      `document.getElementById('${id}').value='${initial}';document.getElementById('${id}').dispatchEvent(new Event('input',{bubbles:true}))`,
    );
    assert.equal(
      await evaluate(`document.getElementById('${id}').disabled`),
      false,
    );
    await evaluate(
      `document.querySelector('.dimension-steps button[data-input="${id}"][data-direction="1"]').click()`,
    );
    assert(
      Math.abs(
        (await evaluate(`Number(document.getElementById('${id}').value)`)) -
          initial -
          0.1,
      ) < 1e-12,
    );
    await evaluate(
      `document.querySelector('.dimension-steps button[data-input="${id}"][data-direction="-1"]').click()`,
    );
    assert.equal(
      await evaluate(`Number(document.getElementById('${id}').value)`),
      initial,
    );
    await evaluate(`document.getElementById('${id}').focus()`);
    for (const [key, code, expected] of [
      ["ArrowUp", 38, initial + 0.1],
      ["ArrowDown", 40, initial],
    ]) {
      await cdp("Input.dispatchKeyEvent", {
        type: "keyDown",
        key,
        code: key,
        windowsVirtualKeyCode: code,
      });
      await cdp("Input.dispatchKeyEvent", {
        type: "keyUp",
        key,
        code: key,
        windowsVirtualKeyCode: code,
      });
      assert(
        Math.abs(
          (await evaluate(`Number(document.getElementById('${id}').value)`)) -
            expected,
        ) < 1e-12,
      );
    }
  }
  for (const [id, value] of Object.entries({
    diameter: 0.8,
    length: 10,
    sampling: 0.15,
    "min-sampling": 0.04,
    stepover: 0.4,
    floor: -3,
  }))
    await dimension(id, value);
  assert.equal(
    await evaluate(
      'document.querySelector(\'.dimension-steps button[data-input="min-sampling"][data-direction="-1"]\').disabled',
    ),
    true,
  );
  await evaluate(
    "document.getElementById('cutter').value='BullCutter';document.getElementById('cutter').dispatchEvent(new Event('change'))",
  );
  await dimension("radius", 0.15);
  await evaluate(
    "document.getElementById('algorithm').value='AdaptiveWaterline';document.getElementById('algorithm').dispatchEvent(new Event('change'))",
  );
  await dimension("height", 2);
  await dimension("z-step", 0.5);
  assert.equal(
    await evaluate("document.getElementById('settings').checkValidity()"),
    true,
  );
  await evaluate(
    "document.getElementById('algorithm').value='AdaptivePathDropCutter';document.getElementById('algorithm').dispatchEvent(new Event('change'));document.getElementById('cutter-preset').value='ball';document.getElementById('cutter-preset').dispatchEvent(new Event('change'))",
  );
  console.log(
    "DIMENSION_STEPS all nine millimetre fields pass with buttons and keyboard",
  );
  await evaluate(
    "document.getElementById('sampling-fine').click();document.getElementById('sampling-presets').scrollIntoView({block:'center'})",
  );
  await screenshot("web-sampling-presets.png");
  const threadedExport = await exportPoints();
  assert.equal(threadedExport.parameters.sampling, 0.05);
  assert.equal(threadedExport.parameters.minSampling, 0.005);
  await evaluate(
    "document.getElementById('export-gcode').click();document.getElementById('gcode-form').requestSubmit()",
  );
  const completeProgram = await exportGcode();
  assert(completeProgram.split("\n").length > 200);
  assert.equal(
    await evaluate("document.getElementById('gcode-preview').value"),
    completeProgram,
  );
  await evaluate("document.getElementById('gcode-close').click()");
  async function checkTip(data, value) {
    await evaluate(
      `document.getElementById('scrub').value='${value}';document.getElementById('scrub').dispatchEvent(new Event('input'))`,
    );
    const points = data.paths.flat(),
      index = Math.floor(((points.length - 1) * value) / 1000),
      point = points[index];
    const shown = await evaluate(
      "['x','y','z'].map(axis=>document.getElementById('tip-'+axis).textContent)",
    );
    assert.deepEqual(
      shown,
      point.map((v) => (Math.abs(v) < 0.0005 ? "0.000" : v.toFixed(3))),
    );
    let segment = 0,
      offset = 0;
    while (offset + data.paths[segment].length <= index) {
      offset += data.paths[segment++].length;
    }
    const context = await evaluate(
      "document.getElementById('tip-context').textContent",
    );
    assert(
      context.startsWith(
        `${data.parameters.algorithm.includes("Path") ? "Pass" : "Contour"} ${segment + 1}/${data.paths.length}`,
      ),
      context,
    );
    const level = data.statistics.levels.findIndex((l) => l.z === point[2]);
    if (level >= 0)
      assert(
        context.includes(
          `Z level ${level + 1}/${data.statistics.levels.length}`,
        ),
        context,
      );
  }
  await checkTip(threadedExport, 0);
  await checkTip(threadedExport, 500);
  await checkTip(threadedExport, 1000);
  await evaluate(
    "document.getElementById('show-cutter').checked=false;document.getElementById('show-cutter').dispatchEvent(new Event('change'))",
  );
  assert.notEqual(
    await evaluate("document.getElementById('tip-z').textContent"),
    "—",
  );
  await checkTip(threadedExport, 0);
  await evaluate("document.getElementById('play').click()");
  await delay(250);
  await evaluate("document.getElementById('play').click()");
  assert.notEqual(
    await evaluate("document.getElementById('scrub').value"),
    "0",
  );
  console.log("TIP raster coordinates and playback pass");

  // Observe the overlay during a full five-level run, then inspect per-level timings.
  await evaluate(
    "document.getElementById('algorithm').value='AdaptiveWaterline';document.getElementById('algorithm').dispatchEvent(new Event('change'));document.getElementById('layers').value='5';document.getElementById('settings').requestSubmit()",
  );
  await waitFor(
    "!document.getElementById('loading-overlay').hidden && document.getElementById('loading-stage').textContent.includes('Computing Z level')",
  );
  assert.equal(
    await evaluate(
      "document.getElementById('model-source').disabled && document.getElementById('upload').disabled",
    ),
    true,
  );
  assert.equal(
    await evaluate(
      "Array.from(document.querySelectorAll('.dimension-steps button')).every(b=>b.disabled)",
    ),
    true,
  );
  assert.equal(
    await evaluate(
      "document.getElementById('height-model-top').disabled && document.getElementById('height-top-step').disabled",
    ),
    true,
  );
  assert.equal(
    await evaluate(
      "Array.from(document.querySelectorAll('#sampling-presets button')).every(b=>b.disabled)",
    ),
    true,
  );
  await evaluate("document.getElementById('sampling-preview').click()");
  assert.deepEqual(await samplingValues(), [0.05, 0.005]);
  await screenshot("tux-web-loading.png");
  await waitFor(idle);
  const full = await exportPoints();
  console.log("FIVE_LEVELS", JSON.stringify(full.statistics));
  assert.equal(full.statistics.levels.length, 5);
  assert.match(
    await evaluate("document.getElementById('detail-stats').textContent"),
    /up to 4 compute threads/,
  );
  assert.equal(
    await evaluate("document.getElementById('layers').hasAttribute('max')"),
    false,
  );
  for (const value of [0, 250, 500, 750, 1000]) await checkTip(full, value);
  console.log("TIP waterline coordinates and level indices pass");
  // A finer run should be cancellable from inside the viewer.
  await evaluate(
    "document.getElementById('layers').value='1000';document.getElementById('length').value='1000';document.getElementById('settings').requestSubmit()",
  );
  await waitFor("!document.getElementById('loading-overlay').hidden");
  await evaluate("document.getElementById('loading-cancel').click()");
  await waitFor(
    "!document.getElementById('run').disabled && document.getElementById('run-label').textContent==='Generate toolpath'",
  );
  assert.equal(
    await evaluate("document.getElementById('loading-overlay').hidden"),
    true,
  );
  for (const [preset, type] of [
    ["flat", "CylCutter"],
    ["bull", "BullCutter"],
    ["cone", "ConeCutter"],
    ["ball", "BallCutter"],
  ]) {
    await evaluate(
      `document.getElementById('cutter-preset').value='${preset}';document.getElementById('cutter-preset').dispatchEvent(new Event('change'))`,
    );
    assert.equal(
      await evaluate("document.getElementById('cutter').value"),
      type,
    );
    console.log("PRESET", preset, await run("PathDropCutter", type));
    await checkCutter(await exportPoints());
  }
  await run("PathDropCutter", "BallCutter", { length: 37.25 });
  await checkCutter(await exportPoints());
  await screenshot("web-full-length-cutter.png");
  console.log(
    "CUTTER_GEOMETRY all four types use full tip-to-top length; Fit includes the cutter",
  );
  await evaluate(
    "document.getElementById('diameter').value='1.2';document.getElementById('diameter').dispatchEvent(new Event('input'))",
  );
  assert.equal(
    await evaluate("document.getElementById('cutter-preset').value"),
    "custom",
  );
  // ASCII tetrahedron positioned well outside the old Tux bounds.
  const faces = [
    [
      [100, 200, 50],
      [110, 200, 50],
      [100, 210, 50],
    ],
    [
      [100, 200, 50],
      [100, 200, 60],
      [110, 200, 50],
    ],
    [
      [100, 200, 50],
      [100, 210, 50],
      [100, 200, 60],
    ],
    [
      [110, 200, 50],
      [100, 200, 60],
      [100, 210, 50],
    ],
  ];
  const ascii =
    "solid uploaded\n" +
    faces
      .map(
        (f) =>
          "facet normal 0 0 0\nouter loop\n" +
          f.map((v) => "vertex " + v.join(" ")).join("\n") +
          "\nendloop\nendfacet",
      )
      .join("\n") +
    "\nendsolid uploaded";
  const asciiPath = join(profile, "web-upload-ascii.stl");
  await writeFile(asciiPath, ascii);
  const binary = Buffer.alloc(84 + faces.length * 50);
  binary.writeUInt32LE(faces.length, 80);
  faces.forEach((f, i) =>
    f
      .flat()
      .forEach((v, j) => binary.writeFloatLE(v, 84 + i * 50 + 12 + j * 4)),
  );
  const binaryPath = join(profile, "web-upload-binary.stl");
  await writeFile(binaryPath, binary);
  const invalidPath = join(profile, "web-invalid.stl");
  await writeFile(invalidPath, "not a valid stl");
  async function upload(file) {
    await evaluate("document.getElementById('stl-file').value=''");
    const doc = await cdp("DOM.getDocument");
    const { nodeId } = await cdp("DOM.querySelector", {
      nodeId: doc.root.nodeId,
      selector: "#stl-file",
    });
    await cdp("DOM.setFileInputFiles", { nodeId, files: [file] });
  }
  const modelReady =
    "!document.getElementById('run').disabled && document.getElementById('status').textContent.startsWith('Model ready')";
  await evaluate(
    "document.getElementById('sampling').value='.037';document.getElementById('sampling').dispatchEvent(new Event('input'));document.getElementById('min-sampling').value='.003';document.getElementById('min-sampling').dispatchEvent(new Event('input'))",
  );
  await upload(asciiPath);
  await waitFor(modelReady);
  assert.equal(
    await evaluate("document.getElementById('stat-points').textContent"),
    "—",
  );
  assert.equal(
    await evaluate("document.getElementById('tip-z').textContent"),
    "—",
  );
  assert.equal(
    await evaluate("document.getElementById('download').disabled"),
    true,
  );
  assert.equal(
    await evaluate("document.getElementById('model-source').value"),
    "upload",
  );
  assert.match(
    await evaluate("document.getElementById('model-info').textContent"),
    /^4 triangles/,
  );
  assert.equal(
    await evaluate("Number(document.getElementById('height').value)"),
    60,
  );
  assert.deepEqual(await samplingValues(), [0.037, 0.003]);
  assert.equal(
    await evaluate(
      "document.querySelectorAll('#sampling-presets [aria-pressed=true]').length",
    ),
    0,
  );
  assert.match(
    await evaluate("document.getElementById('sampling-help').textContent"),
    /^Custom sampling/,
  );
  await evaluate(
    "document.getElementById('algorithm').value='AdaptiveWaterline';document.getElementById('algorithm').dispatchEvent(new Event('change'));document.getElementById('height').value='55';document.getElementById('height-model-top').click()",
  );
  assert.equal(
    await evaluate("Number(document.getElementById('height').value)"),
    60,
  );
  await evaluate("document.getElementById('height-top-step').click()");
  assert.equal(
    await evaluate("Number(document.getElementById('height').value)"),
    59,
  );
  await evaluate(
    "document.getElementById('z-step').value='.7';document.getElementById('height-top-step').click()",
  );
  assert.equal(
    await evaluate("Number(document.getElementById('height').value)"),
    59.3,
  );
  await evaluate(
    "document.getElementById('z-step').value='';document.getElementById('height-top-step').click()",
  );
  assert.match(
    await evaluate("document.getElementById('error').textContent"),
    /positive, finite step down/,
  );
  assert.equal(
    await evaluate("Number(document.getElementById('height').value)"),
    59.3,
  );
  await evaluate(
    "document.getElementById('height-model-top').click();document.getElementById('z-step').value='1'",
  );
  assert.equal(await evaluate("document.getElementById('error').hidden"), true);
  assert.equal(
    await evaluate("Number(document.getElementById('height').value)"),
    60,
  );
  assert.deepEqual(await samplingValues(), [0.037, 0.003]);
  console.log(
    "MODEL_DEFAULTS absolute top Z, both shortcuts, validation and spacing pass",
  );
  await screenshot("tux-web-upload.png");
  console.log("UPLOAD_ASCII", await run("PathDropCutter", "BallCutter"));
  const asciiExport = await exportPoints();
  assert.equal(asciiExport.model, "web-upload-ascii.stl");
  await upload(binaryPath);
  await waitFor(modelReady);
  assert.equal(
    await evaluate("Number(document.getElementById('height').value)"),
    60,
  );
  assert.deepEqual(await samplingValues(), [0.037, 0.003]);
  await run("PathDropCutter", "BallCutter");
  const binaryExport = await exportPoints();
  assert.deepEqual(binaryExport.paths, asciiExport.paths);
  console.log("UPLOAD_BINARY_MATCH");
  assert.deepEqual(
    await evaluate(
      "['center-x','center-y','inner-radius'].map(id=>Number(document.getElementById(id).value))",
    ),
    [105, 205, 0],
  );
  // Exercise real line/arc paths in WASM, including separation of adjacent spans.
  for (const algorithm of ["PathDropCutter", "AdaptivePathDropCutter"]) {
    for (const [pattern, direction] of [
      ["zig", "x"],
      ["zig", "y"],
      ["zigzag", "-y"],
      ["circles", "x"],
    ]) {
      await run(algorithm, "BallCutter", {
        pattern,
        "scan-direction": direction,
        "circle-direction": "cw",
        stepover: 2,
        sampling: 0.25,
        "min-sampling": 0.05,
        "path-filter": "none",
      });
      const output = await exportPoints();
      assert.equal(output.parameters.pattern, pattern);
      assert(output.paths.every((p) => p.length >= 2));
      if (pattern === "circles") {
        for (const path of output.paths.slice(1)) {
          assert.deepEqual(path[0], path.at(-1));
          const radius = Math.hypot(path[0][0] - 105, path[0][1] - 205);
          assert(
            path.every(
              (p) =>
                Math.abs(Math.hypot(p[0] - 105, p[1] - 205) - radius) < 1e-8,
            ),
          );
        }
      } else {
        const axis = direction.endsWith("x") ? 0 : 1;
        output.paths.forEach((path, i) => {
          assert(path.every((p) => p[1 - axis] === path[0][1 - axis]));
          const sign = direction.startsWith("-") ? -1 : 1;
          assert.equal(
            Math.sign(path.at(-1)[axis] - path[0][axis]),
            sign * (pattern === "zigzag" && i % 2 ? -1 : 1),
          );
        });
      }
    }
  }
  await run("AdaptivePathDropCutter", "BallCutter", {
    "path-filter": "line",
    "filter-tolerance": 0.01,
  });
  const filteredCircles = await exportPoints();
  assert(
    filteredCircles.statistics.count < filteredCircles.statistics.rawCount,
  );
  for (const path of filteredCircles.paths.slice(1))
    assert.deepEqual(path[0], path.at(-1));
  await run("PathDropCutter", "BallCutter", {
    "center-x": 104,
    "center-y": 205.5,
    "inner-radius": 1,
    "outer-radius": 4,
    "start-angle": 30,
    "radial-order": "inward",
    "path-filter": "none",
    stepover: 1,
  });
  const customCircles = await exportPoints();
  assert.equal(customCircles.paths.length, 4);
  customCircles.paths.forEach((path, i) => {
    assert(
      Math.abs(Math.hypot(path[0][0] - 104, path[0][1] - 205.5) - (4 - i)) <
        1e-8,
    );
    assert.deepEqual(path[0], path.at(-1));
  });
  for (const algorithm of ["PathDropCutter", "AdaptivePathDropCutter"]) {
    for (const [order, rotation] of [
      ["outward", "cw"],
      ["inward", "ccw"],
    ]) {
      await run(algorithm, "BallCutter", {
        pattern: "spiral",
        "radial-order": order,
        "circle-direction": rotation,
      });
      const output = await exportPoints();
      assert.equal(output.paths.length, 1);
      const path = output.paths[0];
      assert(path.length > 100);
      const radius = (point) => Math.hypot(point[0] - 104, point[1] - 205.5);
      assert(Math.abs(radius(path[0]) - (order === "outward" ? 1 : 4)) < 1e-8);
      assert(
        Math.abs(radius(path.at(-1)) - (order === "outward" ? 4 : 1)) < 1e-8,
      );
      assert(
        Math.abs(
          Math.atan2(path[0][1] - 205.5, path[0][0] - 104) - Math.PI / 6,
        ) < 1e-8,
      );
      assert(path.some((point) => point[2] > output.parameters.floor));
      for (let i = 1; i < path.length; i++) {
        const a = [path[i - 1][0] - 104, path[i - 1][1] - 205.5],
          b = [path[i][0] - 104, path[i][1] - 205.5];
        assert(
          (rotation === "ccw" ? 1 : -1) * (a[0] * b[1] - a[1] * b[0]) >= -1e-10,
        );
      }
    }
  }
  await evaluate(
    "document.getElementById('export-gcode').click();document.getElementById('gcode-form').requestSubmit()",
  );
  const spiralProgram = await exportGcode();
  assert.equal(spiralProgram.match(/^G0 X/gm).length, 1);
  assert.equal(spiralProgram.match(/^G1 Z/gm).length, 1);
  await evaluate(
    "document.getElementById('gcode-close').click();document.getElementById('pattern').scrollIntoView({block:'start'})",
  );
  await screenshot("web-spiral-controls.png");
  await dimension("center-x", 104);
  await dimension("inner-radius", 1);
  await evaluate(
    "document.getElementById('inner-radius').value='4';document.getElementById('settings').requestSubmit()",
  );
  assert.match(
    await evaluate("document.getElementById('error').textContent"),
    /Outer radius/,
  );
  await evaluate("document.getElementById('fit-circles').click()");
  assert.deepEqual(
    await evaluate(
      "['center-x','center-y','inner-radius'].map(id=>Number(document.getElementById(id).value))",
    ),
    [105, 205, 0],
  );
  assert.equal(await evaluate("document.getElementById('error').hidden"), true);
  await evaluate(
    "document.getElementById('start-angle').value='0';document.getElementById('radial-order').value='outward'",
  );
  console.log(
    "RADIAL_OPTIONS custom circles, continuous spirals, rotation/order, XY tolerance, fit shortcut and one-plunge G-code pass",
  );
  for (const algorithm of ["Waterline", "AdaptiveWaterline"]) {
    for (const weave of ["simple", "smart"]) {
      await run(algorithm, "BallCutter", {
        weave,
        height: 59,
        layers: 2,
        "z-step": 1,
        "path-filter": "line",
      });
      const output = await exportPoints();
      assert.equal(output.parameters.weave, weave);
      assert.deepEqual(
        output.statistics.levels.map((l) => l.z),
        [59, 58],
      );
      for (const path of output.paths) assert.deepEqual(path[0], path.at(-1));
    }
  }
  console.log(
    "PATH_OPTIONS line/arc patterns, both weaves and per-path filtering pass",
  );
  const gcodePaths = await exportPoints();
  await evaluate("document.getElementById('export-gcode').click()");
  assert.equal(
    await evaluate("document.getElementById('gcode-dialog').open"),
    true,
  );
  assert.equal(
    await evaluate("Number(document.getElementById('gcode-retract').value)"),
    65,
  );
  await dimension("gcode-retract", 65);
  await evaluate(
    "document.getElementById('gcode-retract').value='59';document.getElementById('gcode-form').requestSubmit()",
  );
  assert.match(
    await evaluate("document.getElementById('gcode-error').textContent"),
    /Retract Z/,
  );
  assert.equal(
    await evaluate("document.getElementById('gcode-download').disabled"),
    true,
  );
  await evaluate(
    "document.getElementById('gcode-retract').value='65';document.getElementById('gcode-feed').value='432.1';document.getElementById('gcode-plunge').value='87.6';document.getElementById('gcode-form').requestSubmit()",
  );
  assert.equal(
    await evaluate("document.getElementById('gcode-error').hidden"),
    true,
  );
  const program = await exportGcode();
  assert(program.includes("G21\nG90\nG17\nG94\n"));
  assert(program.includes("F432.1000") && program.includes("F87.6000"));
  assert(program.includes("M3 S12000\nG4 P1.0000"));
  assert.equal(
    (program.match(/^\(Path /gm) || []).length,
    gcodePaths.paths.length,
  );
  let retract = false;
  for (const line of program.split("\n")) {
    if (/^G0 Z/.test(line)) {
      assert.equal(line, "G0 Z65.0000");
      retract = true;
    }
    if (/^G0 X/.test(line)) assert(retract, "No XY rapid before retract");
    if (/^G1 /.test(line)) retract = false;
  }
  assert(program.endsWith("M9\nM5\nM2\n"));
  await screenshot("web-gcode-desktop.png");
  await evaluate(
    "document.getElementById('gcode-feed').value='500';document.getElementById('gcode-feed').dispatchEvent(new Event('input', {bubbles:true}))",
  );
  assert.equal(
    await evaluate("document.getElementById('gcode-download').disabled"),
    true,
  );
  await evaluate(
    "document.getElementById('gcode-spindle').value='off';document.getElementById('gcode-spindle').dispatchEvent(new Event('change', {bubbles:true}))",
  );
  assert.equal(
    await evaluate(
      "document.getElementById('gcode-rpm').disabled && document.getElementById('gcode-dwell').disabled",
    ),
    true,
  );
  await cdp("Emulation.setDeviceMetricsOverride", {
    width: 390,
    height: 844,
    deviceScaleFactor: 1,
    mobile: true,
  });
  assert.equal(
    await evaluate(
      "document.getElementById('gcode-dialog').scrollWidth <= document.getElementById('gcode-dialog').clientWidth",
    ),
    true,
  );
  await screenshot("web-gcode-mobile.png");
  await evaluate("document.getElementById('gcode-close').click()");
  await cdp("Emulation.setDeviceMetricsOverride", {
    width: 1440,
    height: 960,
    deviceScaleFactor: 1,
    mobile: false,
  });
  await evaluate(
    "(()=>{for(const [id,value] of Object.entries({pattern:'zigzag','scan-direction':'x',weave:'simple','path-filter':'none',height:60})){const e=document.getElementById(id);e.value=value;e.dispatchEvent(new Event('change',{bubbles:true}));}})()",
  );
  console.log(
    "GCODE preview, clearance validation, feeds, spindle and download pass",
  );
  // Starting exactly at the pointed summit must also complete, even if that
  // level has no contour; the following layer supplies a nonempty path.
  for (const algorithm of ["AdaptiveWaterline", "Waterline"]) {
    await run(algorithm, "BallCutter", { layers: 2 });
    const fromTop = await exportPoints();
    assert.deepEqual(
      fromTop.statistics.levels.map((l) => l.z),
      [60, 59],
    );
    assert(fromTop.paths.some((p) => p[0][2] === 59));
  }
  console.log("EXACT_TOP both waterline algorithms handle the model summit");
  // Both waterline algorithms can generate more than the former 20-level cap.
  for (const algorithm of ["AdaptiveWaterline", "Waterline"]) {
    console.log(
      "MANY_LEVELS",
      algorithm,
      await run(algorithm, "BallCutter", {
        layers: 25,
        height: 59,
        "z-step": 0.25,
        sampling: 0.5,
        "min-sampling": 0.1,
      }),
    );
    const many = await exportPoints();
    assert.equal(many.parameters.layers, 25);
    assert.equal(many.statistics.levels.length, 25);
    assert.deepEqual(
      many.statistics.levels.map((l) => l.z),
      Array.from({ length: 25 }, (_, i) => 59 - i * 0.25),
    );
    for (const value of [0, 500, 1000]) await checkTip(many, value);
    await checkTip(many, 1000);
    assert.match(
      await evaluate("document.getElementById('tip-context').textContent"),
      /Z level 25\/25/,
    );
  }
  // Reject fractional/nonpositive counts through the form, and out-of-range
  // arithmetic through the parameter validator, before starting the worker.
  for (const value of ["0", "2.5"]) {
    assert.equal(
      await evaluate(
        `document.getElementById('layers').value='${value}';document.getElementById('layers').checkValidity()`,
      ),
      false,
    );
  }
  for (const value of ["9007199254740992", "1e100"]) {
    await evaluate(
      `document.getElementById('layers').value='${value}';document.getElementById('settings').requestSubmit()`,
    );
    assert.match(
      await evaluate("document.getElementById('error').textContent"),
      /positive whole number/,
    );
    assert.equal(
      await evaluate("document.getElementById('loading-overlay').hidden"),
      true,
    );
  }
  await evaluate(
    "document.getElementById('layers').value='25';document.getElementById('z-step').value='1e308';document.getElementById('settings').requestSubmit()",
  );
  assert.match(
    await evaluate("document.getElementById('error').textContent"),
    /final Z level must be finite/,
  );
  assert.equal(
    await evaluate("document.getElementById('loading-overlay').hidden"),
    true,
  );
  console.log("LEVEL_VALIDATION pass");
  await evaluate(
    "document.getElementById('z-step').value='.25';document.getElementById('length').value='1';document.getElementById('settings').requestSubmit()",
  );
  assert.match(
    await evaluate("document.getElementById('error').textContent"),
    /at least 7.0 mm/,
  );
  assert.equal(
    await evaluate("document.getElementById('loading-overlay').hidden"),
    true,
  );
  await run("Waterline", "BallCutter", { length: 10 });
  assert.equal((await exportPoints()).statistics.levels.length, 25);
  console.log(
    "SYNTHETIC_REACH_CHECK rejects an undersized cutter and recovers",
  );
  const previousModelName = await evaluate(
    "document.getElementById('model-name').textContent",
  );
  await upload(invalidPath);
  // Deliberate validation error must preserve the loaded model and old toolpath.
  await delay(500);
  assert.equal(
    await evaluate("document.getElementById('error').hidden"),
    false,
  );
  assert.equal(
    await evaluate("document.getElementById('model-name').textContent"),
    previousModelName,
  );
  assert.equal(
    await evaluate("document.getElementById('loading-overlay').hidden"),
    true,
  );
  await evaluate(
    "document.getElementById('sampling').value='.032';document.getElementById('sampling').dispatchEvent(new Event('input'));document.getElementById('min-sampling').value='.004';document.getElementById('min-sampling').dispatchEvent(new Event('input'))",
  );
  await evaluate(
    "document.getElementById('model-source').value='tux';document.getElementById('model-source').dispatchEvent(new Event('change'))",
  );
  await waitFor(modelReady);
  assert.deepEqual(await samplingValues(), [0.032, 0.004]);
  assert.equal(
    await evaluate(
      "document.querySelectorAll('#sampling-presets [aria-pressed=true]').length",
    ),
    0,
  );
  console.log(
    "SAMPLING_PERSISTENCE custom inputs and presets survive uploads and switching models",
  );
  await evaluate("document.getElementById('sampling-fine').click()");
  assert.equal(await evaluate("document.getElementById('layers').value"), "1");
  await evaluate(
    "document.getElementById('cutter-preset').value='ball';document.getElementById('cutter-preset').dispatchEvent(new Event('change'))",
  );
  console.log("PREVIEW", await run("AdaptiveWaterline", "BallCutter"));
  assert.equal((await exportPoints()).statistics.levels.length, 1);
  await screenshot("tux-web-waterline.png");
  await cdp("Emulation.setDeviceMetricsOverride", {
    width: 390,
    height: 844,
    deviceScaleFactor: 1,
    mobile: true,
  });
  await delay(300);
  await evaluate("document.getElementById('fit-view').click()");
  assert.equal(await evaluate("window.cutterFits()"), true);
  assert.equal(
    await evaluate("document.documentElement.scrollWidth <= innerWidth"),
    true,
  );
  await screenshot("tux-web-mobile.png");
  await evaluate(
    "document.getElementById('sampling-presets').scrollIntoView({block:'center'})",
  );
  assert.equal(
    await evaluate(
      "Array.from(document.querySelectorAll('#sampling-presets button')).every(b=>b.scrollWidth<=b.clientWidth)",
    ),
    true,
  );
  assert.equal(
    await evaluate(
      `(()=>{const context=document.createElement('canvas').getContext('2d');return ['sampling','min-sampling'].every(id=>{const input=document.getElementById(id),style=getComputedStyle(input);context.font=style.font;return context.measureText(input.value).width<input.clientWidth-parseFloat(style.paddingLeft)-parseFloat(style.paddingRight)})})()`,
    ),
    true,
  );
  await screenshot("web-sampling-mobile.png");
  await cdp("Emulation.setDeviceMetricsOverride", {
    width: 1440,
    height: 960,
    deviceScaleFactor: 1,
    mobile: false,
  });
  // A Pages-style project subpath without isolation headers uses serial WASM.
  await cdp("Page.navigate", { url: base + "/opencamlib/" });
  await waitFor(idle);
  assert.equal(await evaluate("crossOriginIsolated"), false);
  assert.equal(await evaluate("document.getElementById('threads').value"), "1");
  const serialExport = await exportPoints();
  assert.deepEqual(serialExport.paths, threadedExport.paths);
  console.log(
    "PAGES: project subpath loads; serial and threaded coordinates match exactly",
  );
  console.log("SERIAL", await run("PathDropCutter", "BallCutter"));
  await run("PathDropCutter", "BallCutter", {
    pattern: "circles",
    "path-filter": "line",
    stepover: 1,
    sampling: 0.25,
  });
  assert((await exportPoints()).statistics.rawCount > 0);
  await run("Waterline", "BallCutter", { weave: "smart" });
  assert(
    (await exportPoints()).paths.every((p) =>
      p[0].every((v, i) => v === p.at(-1)[i]),
    ),
  );
  console.log("SERIAL_OPTIONS circles, Smart weave and native filtering pass");
  // The delivered file must also work without any local HTTP server.
  await cdp("Page.navigate", {
    url: pathToFileURL(resolve("examples/web/index.html")).href,
  });
  await waitFor(idle);
  console.log(
    "FILE",
    await evaluate("document.getElementById('status').textContent"),
  );
  assert.equal(exceptions.length, 0, JSON.stringify(exceptions));
  const external = [...new Set(requests.filter((x) => x.startsWith("https:")))];
  assert(
    external.every((x) =>
      x.startsWith("https://cdn.jsdelivr.net/npm/three@0.180.0/"),
    ),
    JSON.stringify(external),
  );
  console.log("PASS", JSON.stringify({ external, screenshots }));
} catch (e) {
  console.error(e);
  console.error(logs.slice(-5000));
  process.exitCode = 1;
} finally {
  socket?.close();
  chrome.kill("SIGTERM");
  const timer = setTimeout(() => chrome.kill("SIGKILL"), 5000);
  await new Promise((r) => {
    if (!chrome.pid || chrome.exitCode !== null || chrome.signalCode !== null)
      r();
    else chrome.once("exit", r);
  });
  clearTimeout(timer);
  chrome.stderr.destroy();
  server.closeAllConnections();
  server.close();
  await delay(300);
  await rm(profile, {
    recursive: true,
    force: true,
    maxRetries: 5,
    retryDelay: 200,
  });
}
