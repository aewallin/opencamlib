// Pure path-planning and G-code checks; no browser or WASM build required.
import assert from "node:assert/strict";
import test from "node:test";
import "../examples/web/cam.js";

const { fitCircularBounds, planPaths, splitPath, writeGcode } = globalThis.CAM;
const bounds = { min: [-2, -1, -3], max: [2, 1, 2] };
const parameters = {
  pattern: "zigzag",
  scanDirection: "x",
  circleDirection: "ccw",
  stepover: 1,
  diameter: 1,
  floor: -3,
  ...fitCircularBounds(bounds, 1),
  radialOrder: "outward",
  startAngle: 0,
  spiralTolerance: 0.001,
};
const settings = {
  retract: 8,
  feed: 300,
  plunge: 80,
  spindle: "cw",
  rpm: 12000,
  dwell: 1.5,
  coolant: "off",
  decimals: 4,
  workOffset: "G54",
};
const result = {
  model: "test.stl",
  bounds,
  params: {
    algorithm: "AdaptivePathDropCutter",
    cutter: "BallCutter",
    diameter: 1,
  },
  coordinates: new Float64Array([
    0, 0, 1, 1, 0, 0.5, 1, 0, -1, 1, 1, -1, -2, -1, 0, -1, -1, 1,
  ]),
  offsets: new Uint32Array([0, 4, 6]),
};

test("raster patterns cover both edges and preserve scan directions", () => {
  for (const direction of ["x", "-x", "y", "-y"]) {
    const axis = direction.endsWith("x") ? 0 : 1,
      cross = 1 - axis;
    for (const pattern of ["zig", "zigzag"]) {
      const spans = planPaths(
        { ...parameters, pattern, scanDirection: direction },
        bounds,
      );
      assert.equal(spans[0].start[cross], bounds.min[cross] - 0.5);
      assert.equal(spans.at(-1).start[cross], bounds.max[cross] + 0.5);
      spans.forEach((span, i) => {
        const sign = direction.startsWith("-") ? -1 : 1;
        const alternating = pattern === "zigzag" && i % 2 ? -1 : 1;
        assert.equal(
          Math.sign(span.end[axis] - span.start[axis]),
          sign * alternating,
        );
        assert.equal(span.start[cross], span.end[cross]);
        assert.equal(span.start[2], parameters.floor);
        assert.equal(span.path, i);
        if (i)
          assert(
            span.start[cross] - spans[i - 1].start[cross] <=
              parameters.stepover,
          );
      });
    }
  }
});

test("circular paths contain a center pass and complete paired semicircles", () => {
  for (const direction of ["cw", "ccw"]) {
    const spans = planPaths(
      { ...parameters, pattern: "circles", circleDirection: direction },
      bounds,
    );
    assert(!spans[0].center);
    assert(spans[0].start[0] < 0 && spans[0].end[0] > 0);
    for (let i = 1; i < spans.length; i += 2) {
      assert.equal(spans[i].ccw, direction === "ccw");
      assert.deepEqual(spans[i].start, spans[i + 1].end);
      assert.deepEqual(spans[i].end, spans[i + 1].start);
      assert.equal(spans[i].path, spans[i + 1].path);
    }
    assert(Math.abs(spans.at(-1).end[0] - (Math.hypot(2, 1) + 0.5)) < 1e-12);
  }
});

test("custom circles preserve the center, exclusion radius, angle and inward order", () => {
  const p = {
    ...parameters,
    pattern: "circles",
    centerX: 12,
    centerY: -7,
    innerRadius: 2,
    outerRadius: 5,
    startAngle: 90,
    radialOrder: "inward",
  };
  const spans = planPaths(p, bounds);
  const radii = spans
    .filter((_, i) => i % 2 === 0)
    .map((span) => Math.hypot(span.start[0] - 12, span.start[1] + 7));
  assert.deepEqual(radii, [5, 4, 3, 2]);
  spans.forEach((span) => assert.deepEqual(span.center, [12, -7, -3]));
  assert.deepEqual(spans[0].start, [12, -2, -3]);
  const fitted = fitCircularBounds(
    { min: [100, 200, 50], max: [110, 210, 60] },
    2,
  );
  assert.deepEqual(fitted, {
    centerX: 105,
    centerY: 205,
    innerRadius: 0,
    outerRadius: Math.hypot(5, 5) + 1,
  });
});

test("spirals keep their pitch and rotation in both radial orders within XY tolerance", () => {
  for (const circleDirection of ["cw", "ccw"]) {
    for (const radialOrder of ["outward", "inward"]) {
      for (const innerRadius of [0, 2]) {
        const p = {
          ...parameters,
          pattern: "spiral",
          centerX: 12,
          centerY: -7,
          innerRadius,
          outerRadius: 5,
          startAngle: 90,
          circleDirection,
          radialOrder,
        };
        const spans = planPaths(p, bounds);
        const inward = radialOrder === "inward";
        const firstRadius = inward ? 5 : innerRadius;
        const lastRadius = inward ? innerRadius : 5;
        const sweep = ((5 - innerRadius) / p.stepover) * 2 * Math.PI;
        const at = (t) => {
          const r = firstRadius + (lastRadius - firstRadius) * t;
          const angle =
            Math.PI / 2 + (circleDirection === "ccw" ? 1 : -1) * sweep * t;
          return [12 + r * Math.cos(angle), -7 + r * Math.sin(angle), -3];
        };
        assert.deepEqual(spans[0].start, at(0));
        spans.forEach((span, i) => {
          assert.equal(span.path, 0);
          if (i) assert.deepEqual(span.start, spans[i - 1].end);
          for (const fraction of [0, 0.25, 0.5, 0.75, 1]) {
            const exact = at((i + fraction) / spans.length);
            const chord = span.start.map(
              (v, k) => v + fraction * (span.end[k] - v),
            );
            assert(
              Math.hypot(chord[0] - exact[0], chord[1] - exact[1]) <=
                p.spiralTolerance + 1e-12,
            );
          }
        });
        const path = splitPath(
          spans.flatMap((span) => [span.start, span.end]),
          spans,
        );
        assert.equal(path.length, 1);
        assert.equal(path[0].length, spans.length + 1);
        const output = writeGcode(
          {
            ...result,
            coordinates: path[0].flat(),
            offsets: [0, path[0].length],
          },
          settings,
        );
        assert.equal(output.paths, 1);
        assert.equal(output.text.match(/^G0 X/gm).length, 1);
        assert.equal(output.text.match(/^G1 Z/gm).length, 1);
      }
    }
  }
});

test("radial patterns reject invalid dimensions and impractical spiral resolution", () => {
  for (const pattern of ["circles", "spiral"]) {
    for (const changed of [
      { centerX: NaN },
      { centerY: Infinity },
      { startAngle: NaN },
      { innerRadius: -1 },
      { innerRadius: 4, outerRadius: 4 },
      { outerRadius: 0 },
      { circleDirection: "unknown" },
      { radialOrder: "unknown" },
    ])
      assert.throws(() =>
        planPaths({ ...parameters, pattern, ...changed }, bounds),
      );
  }
  for (const changed of [
    { spiralTolerance: 0 },
    { spiralTolerance: NaN },
    { stepover: 1e-12 },
    { spiralTolerance: 1e-30 },
  ])
    assert.throws(() =>
      planPaths({ ...parameters, pattern: "spiral", ...changed }, bounds),
    );
});

test("span boundaries keep separate paths apart even when their Y values match", () => {
  const spans = [
    { start: [1, 0, -3], end: [-1, 0, -3], path: 0 },
    { start: [-1, 0, -3], end: [1, 0, -3], path: 0 },
    { start: [2, 0, -3], end: [-2, 0, -3], path: 1 },
  ];
  const points = [
    [1, 0, 2],
    [0, 1, 3],
    [-1, 0, 2],
    [-1, 0, 2],
    [0, -1, 3],
    [1, 0, 2],
    [2, 0, 1],
    [-2, 0, 1],
  ];
  assert.deepEqual(splitPath(points, spans), [
    points.slice(0, 3).concat(points.slice(4, 6)),
    points.slice(6),
  ]);
  assert.throws(() => splitPath(points.slice(1), spans), /input spans/);
  assert.throws(() => splitPath(points.slice(0, -1), spans), /endpoint/);
  assert.throws(
    () => splitPath([...points, [9, 9, 9]], spans),
    /unexpected points/,
  );
  const rounded = points.map((point) => [...point]);
  rounded.at(-1)[0] += Number.EPSILON;
  assert.equal(splitPath(rounded, spans).length, 2);
  assert.equal(
    splitPath(
      [
        [-1000, 0, 0],
        [-1000 + 1000.1, 0, 0],
      ],
      [{ start: [-1000, 0, 0], end: [0.1, 0, 0], path: 0 }],
    ).length,
    1,
  );
});

test("G-code retracts before all XY rapids and preserves each cutting segment", () => {
  const output = writeGcode(result, settings);
  assert.equal(output.paths, 2);
  assert(output.text.includes("G21\nG90\nG17\nG94\nG40\nG80\nG54\n"));
  let z,
    feed,
    cutting = [];
  const paths = [];
  for (const line of output.text.split("\n")) {
    if (!/^G[01] /.test(line)) continue;
    const words = Object.fromEntries(
      [...line.matchAll(/([XYZF])(-?\d+(?:\.\d+)?)/g)].map((m) => [
        m[1],
        Number(m[2]),
      ]),
    );
    if (line.startsWith("G0")) {
      if ("X" in words || "Y" in words) {
        assert.equal(z, settings.retract, "XY travel must stay at retract Z");
        assert(!("Z" in words), "Never combine retraction and XY travel");
        cutting = [[words.X, words.Y]];
      } else {
        assert.equal(words.Z, settings.retract);
        if (cutting.length) {
          paths.push(cutting);
          cutting = [];
        }
      }
    } else {
      feed = words.F ?? feed;
      if ("X" in words) {
        const previous = cutting.at(-1);
        const verticalDown =
          words.X === previous[0] &&
          words.Y === previous[1] &&
          words.Z < previous[2];
        assert.equal(feed, verticalDown ? settings.plunge : settings.feed);
        cutting.push([words.X, words.Y, words.Z]);
      } else {
        assert.equal(feed, settings.plunge);
        cutting[0].push(words.Z);
      }
    }
    z = words.Z ?? z;
  }
  assert.deepEqual(paths, [
    [
      [0, 0, 1],
      [1, 0, 0.5],
      [1, 0, -1],
      [1, 1, -1],
    ],
    [
      [-2, -1, 0],
      [-1, -1, 1],
    ],
  ]);
  assert(output.text.endsWith("G0 Z8.0000\nM9\nM5\nM2\n"));
});

test("clearance validation accounts for model, path heights, and output rounding", () => {
  for (const retract of [1, 2, 2.00001, NaN, Infinity])
    assert.throws(() => writeGcode(result, { ...settings, retract }));
  const aboveModel = {
    ...result,
    coordinates: new Float64Array(result.coordinates),
  };
  aboveModel.coordinates[2] = 9;
  assert.throws(() => writeGcode(aboveModel, settings), /Retract Z/);
});

test("invalid feeds, settings, geometry and segment boundaries cannot produce a program", () => {
  for (const changed of [
    { feed: 0 },
    { plunge: -1 },
    { feed: 0.00001 },
    { feed: Infinity },
    { rpm: 1.2 },
    { dwell: -1 },
    { decimals: 2 },
    { workOffset: "G54\nG0 X0" },
  ])
    assert.throws(() => writeGcode(result, { ...settings, ...changed }));
  for (const offsets of [
    [1, 6],
    [0, 7, 6],
    [0, -1, 6],
    [0, 1.5, 6],
  ])
    assert.throws(() => writeGcode({ ...result, offsets }, settings));
  assert.throws(() =>
    writeGcode(
      { ...result, coordinates: [NaN, 0, 0], offsets: [0, 1] },
      settings,
    ),
  );
  assert.throws(() =>
    writeGcode({ ...result, coordinates: [], offsets: [0] }, settings),
  );
});

test("spindle, coolant, work offset and precision settings are reflected in output", () => {
  const output = writeGcode(result, {
    ...settings,
    spindle: "ccw",
    coolant: "flood",
    workOffset: "G55",
    decimals: 3,
  }).text;
  assert(output.includes("G55\n"));
  assert(output.includes("M4 S12000\nG4 P1.500\nM8\n"));
  assert(output.includes("G1 Z1.000 F80.000\n"));
  const manual = writeGcode(result, { ...settings, spindle: "off" }).text;
  assert(!/^M[345]\b|^G4\b/m.test(manual));
});

test("filenames cannot inject G-code or unbalanced comments", () => {
  const output = writeGcode(
    { ...result, model: "bad)\nG0 Z-100\n(M3;\u0000.stl" },
    settings,
  ).text;
  assert(!output.includes("\nG0 Z-100\n"));
  for (const line of output.split("\n").filter((s) => s.startsWith("(")))
    assert.match(line, /^\([^()\r\n]*\)$/);
});
