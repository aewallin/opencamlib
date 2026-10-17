let cam, surface;
const release = (value) => value?.delete();
function points(vector) {
  const result = [];
  try {
    for (let i = 0; i < vector.size(); i++) {
      const p = vector.get(i);
      try {
        result.push([p.x, p.y, p.z]);
      } finally {
        p.delete();
      }
    }
  } finally {
    vector.delete();
  }
  return result;
}
function readLoops(vector) {
  const result = [];
  try {
    for (let i = 0; i < vector.size(); i++) {
      const loop = points(vector.get(i));
      if (loop.length > 1) {
        if (loop[0].some((v, j) => v !== loop[loop.length - 1][j]))
          loop.push([...loop[0]]);
        result.push(loop);
      }
    }
  } finally {
    vector.delete();
  }
  return result;
}
function filterSegment(segment, tolerance) {
  const same = (a, b) => a.every((v, i) => v === b[i]);
  segment = segment.filter((point, i) => !i || !same(point, segment[i - 1]));
  if (segment.length < 3) return segment;
  // LineCLFilter expects an open chain. Preserve closed loops by filtering
  // two open halves, so it never fits a line between identical endpoints.
  if (same(segment[0], segment[segment.length - 1])) {
    const middle = Math.floor(segment.length / 2);
    return filterSegment(segment.slice(0, middle + 1), tolerance).concat(
      filterSegment(segment.slice(middle), tolerance).slice(1),
    );
  }
  const filter = new cam.LineCLFilter();
  try {
    filter.setTolerance(tolerance);
    for (const coordinates of segment) {
      const point = new cam.CLPoint(...coordinates);
      try {
        filter.addCLPoint(point);
      } finally {
        point.delete();
      }
    }
    filter.run();
    return points(filter.getCLPoints());
  } finally {
    filter.delete();
  }
}
self.onmessage = async ({ data }) => {
  try {
    if (data.type === "init") {
      cam = await ocl({
        mainScriptUrlOrBlob: data.url,
        print() {},
        printErr(message) {
          console.warn(message);
        },
      });
      surface = new cam.STLSurf();
      const input = new Float32Array(data.positions);
      const count = input.length / 9;
      for (let i = 0; i < count; i++) {
        const p = [];
        let triangle;
        try {
          for (let v = 0; v < 3; v++) {
            const o = i * 9 + v * 3;
            p.push(new cam.Point(input[o], input[o + 1], input[o + 2]));
          }
          triangle = new cam.Triangle(...p);
          surface.addTriangle(triangle);
        } finally {
          release(triangle);
          p.forEach(release);
        }
      }
      self.postMessage({ type: "ready", maxThreads: cam.maxThreads() });
      return;
    }
    if (data.type !== "run") return;
    const { params: p, bounds: b } = data;
    const start = performance.now();
    let cutter, operation, path;
    let result,
      computeMs = 0;
    const levels = [];
    try {
      // ConeCutter's final argument is shaft length; the UI consistently uses
      // total length from the tip, including the cone.
      cutter =
        p.cutter === "BullCutter"
          ? new cam.BullCutter(p.diameter, p.radius, p.length)
          : p.cutter === "ConeCutter"
            ? new cam.ConeCutter(
                p.diameter,
                (p.angle * Math.PI) / 360,
                p.length - p.diameter / 2 / Math.tan((p.angle * Math.PI) / 360),
              )
            : new cam[p.cutter](p.diameter, p.length);
      operation = new cam[p.algorithm]();
      operation.setSTL(surface);
      operation.setCutter(cutter);
      operation.setSampling(p.sampling);
      operation.setThreads(p.threads);
      if (p.algorithm.startsWith("Adaptive"))
        operation.setMinSampling(p.minSampling);
      if (p.algorithm.includes("Path")) {
        path = new cam.Path();
        const spans = CAM.planPaths(p, b);
        for (const span of spans) {
          const start = new cam.Point(...span.start),
            end = new cam.Point(...span.end);
          let nativeSpan, center;
          try {
            if (span.center) {
              center = new cam.Point(...span.center);
              nativeSpan = new cam.Arc(start, end, center, span.ccw);
              path.appendArc(nativeSpan);
            } else {
              nativeSpan = new cam.Line(start, end);
              path.appendLine(nativeSpan);
            }
          } finally {
            release(nativeSpan);
            release(center);
            start.delete();
            end.delete();
          }
        }
        operation.setPath(path);
        operation.setZ(p.floor);
        self.postMessage({
          type: "progress",
          text:
            p.pattern === "spiral"
              ? "Computing continuous spiral…"
              : `Computing ${spans[spans.length - 1].path + 1} ${p.pattern === "circles" ? "circular paths" : "raster passes"}…`,
        });
        const t = performance.now();
        operation.run();
        computeMs = performance.now() - t;
        result = CAM.splitPath(points(operation.getPoints()), spans);
      } else {
        result = [];
        for (let layer = 0; layer < p.layers; layer++) {
          const z = p.height - layer * p.zStep;
          self.postMessage({
            type: "progress",
            text: `Computing Z level ${layer + 1} of ${p.layers} · Z ${z.toFixed(3)} mm`,
            completed: layer,
            total: p.layers,
          });
          operation.setZ(z);
          const t = performance.now();
          if (p.weave === "smart") operation.run2();
          else operation.run();
          const ms = performance.now() - t;
          computeMs += ms;
          levels.push({ z, computeMs: ms });
          result.push(...readLoops(operation.getLoops()));
          self.postMessage({
            type: "progress",
            text: `Completed ${layer + 1} of ${p.layers} levels`,
            completed: layer + 1,
            total: p.layers,
          });
        }
      }
      const rawCount = result.reduce(
        (count, segment) => count + segment.length,
        0,
      );
      if (p.pathFilter === "line") {
        self.postMessage({
          type: "progress",
          text: "Filtering points within each path…",
        });
        result = result.map((segment) =>
          filterSegment(segment, p.filterTolerance),
        );
      }
      const totalMs = performance.now() - start;
      let length = 0,
        count = 0;
      for (const segment of result) {
        count += segment.length;
        for (let i = 1; i < segment.length; i++)
          length += Math.hypot(
            ...segment[i].map((v, j) => v - segment[i - 1][j]),
          );
      }
      const offsets = new Uint32Array(result.length + 1),
        coordinates = new Float64Array(count * 3);
      let cursor = 0;
      result.forEach((segment, i) => {
        offsets[i] = cursor;
        for (const point of segment) {
          coordinates.set(point, cursor * 3);
          cursor++;
        }
      });
      offsets[result.length] = cursor;
      self.postMessage(
        {
          type: "result",
          coordinates,
          offsets,
          stats: {
            computeMs,
            totalMs,
            length,
            count,
            rawCount,
            paths: result.length,
            threads: operation.getThreads(),
            levels,
          },
        },
        [coordinates.buffer, offsets.buffer],
      );
    } finally {
      release(operation);
      release(path);
      release(cutter);
    }
  } catch (error) {
    self.postMessage({
      type: "error",
      message: error.message || String(error),
    });
  }
};
