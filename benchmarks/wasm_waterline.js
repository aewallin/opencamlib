// Reproduce the web demo's five-level adaptive-waterline workload in Node.
// Build with OCL_WASM_ENVIRONMENT=web,worker,node. Use the same STL in each run.
// Usage: node benchmarks/wasm_waterline.js <ocl.js> <model.stl> [threads=4] [runs=3]
const fs = require("fs");
const path = require("path");
const { createHash } = require("crypto");

async function main() {
  const [modulePath, modelPath, threadsArg = "4", runsArg = "3"] =
    process.argv.slice(2);
  const threads = Number(threadsArg),
    runs = Number(runsArg);
  if (
    !modulePath ||
    !modelPath ||
    !Number.isInteger(threads) ||
    threads < 1 ||
    !Number.isInteger(runs) ||
    runs < 1
  ) {
    throw new Error(
      "Usage: node benchmarks/wasm_waterline.js <ocl.js> <model.stl> [threads=4] [runs=3]",
    );
  }
  const cam = await require(path.resolve(modulePath))({
    print() {},
    printErr: console.warn,
  });
  cam.FS.writeFile("/benchmark.stl", fs.readFileSync(modelPath));
  let surface, reader, cutter, operation;
  try {
    surface = new cam.STLSurf();
    reader = new cam.STLReader("/benchmark.stl", surface);
    cutter = new cam.BallCutter(0.8, 10);
    operation = new cam.AdaptiveWaterline();
    const start = performance.now();
    operation.setSTL(surface);
    operation.setCutter(cutter);
    operation.setThreads(threads);
    operation.setSampling(0.15);
    operation.setMinSampling(0.04);
    const setupMs = performance.now() - start;
    for (let run = 0; run < runs; ++run) {
      const levels = [],
        hash = createHash("sha256");
      for (const z of [2, 1.5, 1, 0.5, 0]) {
        operation.setZ(z);
        const start = performance.now();
        operation.run();
        const computeMs = performance.now() - start;
        const loops = operation.getLoops();
        let points = 0;
        try {
          hash.update(`z=${z};loops=${loops.size()};`);
          for (let i = 0; i < loops.size(); ++i) {
            const loop = loops.get(i);
            try {
              hash.update(`points=${loop.size()};`);
              points += loop.size();
              for (let j = 0; j < loop.size(); ++j) {
                const p = loop.get(j);
                try {
                  if (![p.x, p.y, p.z].every(Number.isFinite))
                    throw new Error("Non-finite toolpath coordinate");
                  hash.update(JSON.stringify([p.x, p.y, p.z]));
                } finally {
                  p.delete();
                }
              }
            } finally {
              loop.delete();
            }
          }
          levels.push({ z, computeMs, points, contours: loops.size() });
        } finally {
          loops.delete();
        }
      }
      console.log(
        JSON.stringify({
          run: run + 1,
          threads: operation.getThreads(),
          setupMs,
          computeMs: levels.reduce((sum, level) => sum + level.computeMs, 0),
          levels,
          coordinateHash: hash.digest("hex"),
        }),
      );
    }
  } finally {
    for (const object of [operation, cutter, reader, surface]) object?.delete();
  }
}
main().catch((error) => {
  console.error(error);
  process.exitCode = 1;
});
