// Shared by the page and its calculation worker; build.py embeds this in the HTML.
globalThis.CAM = (() => {
  function fitCircularBounds(bounds, diameter) {
    return {
      centerX: (bounds.min[0] + bounds.max[0]) / 2,
      centerY: (bounds.min[1] + bounds.max[1]) / 2,
      innerRadius: 0,
      outerRadius:
        Math.hypot(
          (bounds.max[0] - bounds.min[0]) / 2,
          (bounds.max[1] - bounds.min[1]) / 2,
        ) +
        diameter / 2,
    };
  }

  function validateCircularSettings(p) {
    if (
      ![p.centerX, p.centerY, p.innerRadius, p.outerRadius, p.startAngle].every(
        Number.isFinite,
      )
    )
      throw new Error(
        "Center coordinates, radii and start angle must be finite.",
      );
    if (p.innerRadius < 0 || p.outerRadius <= p.innerRadius)
      throw new Error(
        "Outer radius must be larger than inner radius, which must be nonnegative.",
      );
    if (
      !["cw", "ccw"].includes(p.circleDirection) ||
      !["outward", "inward"].includes(p.radialOrder)
    )
      throw new Error("Choose a supported rotation and radial order.");
    if (
      p.pattern === "spiral" &&
      (!Number.isFinite(p.spiralTolerance) || p.spiralTolerance <= 0)
    )
      throw new Error("Spiral XY tolerance must be finite and positive.");
  }

  function planPaths(p, bounds) {
    if (!Number.isFinite(p.stepover) || p.stepover <= 0)
      throw new Error("Pass spacing must be finite and positive.");
    const spans = [];
    const margin = p.diameter / 2;
    if (["circles", "spiral"].includes(p.pattern)) {
      validateCircularSettings(p);
      const center = [p.centerX, p.centerY, p.floor];
      const angle = ((p.startAngle % 360) * Math.PI) / 180;
      const point = (r, a = angle) => [
        center[0] + r * Math.cos(a),
        center[1] + r * Math.sin(a),
        p.floor,
      ];
      const inward = p.radialOrder === "inward";
      const width = p.outerRadius - p.innerRadius;
      if (p.pattern === "spiral") {
        // Archimedean spiral: one revolution changes the radius by one stepover.
        const slope = p.stepover / (2 * Math.PI);
        const sweep = width / slope;
        // Linear interpolation error <= max(|P''|) * deltaAngle^2 / 8.
        // For P(t) = r(t) [cos(t), sin(t)], |P''| = hypot(r, 2 * slope).
        const step = Math.min(
          Math.PI / 2,
          Math.sqrt(
            (8 * p.spiralTolerance) / Math.hypot(p.outerRadius, 2 * slope),
          ),
        );
        const count = Math.ceil(sweep / step);
        if (!Number.isSafeInteger(count) || count < 1 || count > 1000000)
          throw new Error(
            "Spiral requires too many segments. Increase pass spacing or spiral XY tolerance.",
          );
        let start = point(inward ? p.outerRadius : p.innerRadius);
        for (let i = 1; i <= count; i++) {
          const radius = inward
            ? p.outerRadius - (width * i) / count
            : p.innerRadius + (width * i) / count;
          const end = point(
            radius,
            angle +
              ((p.circleDirection === "ccw" ? 1 : -1) * sweep * i) / count,
          );
          spans.push({ start, end, path: 0 });
          start = end;
        }
      } else {
        const rings = Math.ceil(width / p.stepover);
        if (!Number.isSafeInteger(rings) || rings < 1 || rings > 500000)
          throw new Error("Pass spacing requires too many circular paths.");
        for (let path = 0; path <= rings; path++) {
          const i = inward ? rings - path : path;
          const r = p.innerRadius + (width * i) / rings;
          if (r === 0) {
            // Cover the center only when no inner exclusion radius is requested.
            spans.push({
              start: point(-width / rings),
              end: point(width / rings),
              path,
            });
          } else {
            const right = point(r),
              left = point(-r);
            const arc = { center, ccw: p.circleDirection === "ccw", path };
            spans.push(
              { ...arc, start: right, end: left },
              { ...arc, start: left, end: right },
            );
          }
        }
      }
    } else {
      const axis = p.scanDirection.endsWith("y") ? 1 : 0;
      const cross = 1 - axis;
      const low = bounds.min[cross] - margin,
        high = bounds.max[cross] + margin;
      const passes = Math.ceil((high - low) / p.stepover) + 1;
      if (!Number.isSafeInteger(passes) || passes < 2)
        throw new Error("Pass spacing requires too many raster passes.");
      for (let i = 0; i < passes; i++) {
        const offset = low + (i * (high - low)) / (passes - 1);
        const start = [0, 0, p.floor],
          end = [0, 0, p.floor];
        const reverse =
          p.scanDirection.startsWith("-") !==
          (p.pattern === "zigzag" && i % 2 === 1);
        start[cross] = end[cross] = offset;
        start[axis] = reverse
          ? bounds.max[axis] + margin
          : bounds.min[axis] - margin;
        end[axis] = reverse
          ? bounds.min[axis] - margin
          : bounds.max[axis] + margin;
        spans.push({ start, end, path: i });
      }
    }
    return spans;
  }

  // Native path operations return each span in order, including both endpoints.
  // Keep that structure instead of guessing boundaries from constant X or Y.
  function splitPath(points, spans) {
    const paths = [];
    let cursor = 0;
    // Line interpolation can round an endpoint by a few floating-point ULPs.
    const sameXY = (a, b, span) =>
      a &&
      [0, 1].every(
        (i) =>
          Math.abs(a[i] - b[i]) <=
          16 *
            Number.EPSILON *
            Math.max(
              1,
              Math.abs(a[i]),
              Math.abs(b[i]),
              Math.abs(span.start[i]),
              Math.abs(span.end[i]),
            ),
      );
    for (const span of spans) {
      if (!sameXY(points[cursor], span.start, span))
        throw new Error("The computed path does not match its input spans.");
      const segment = paths[span.path] || (paths[span.path] = []);
      let point;
      do {
        point = points[cursor++];
        if (!point)
          throw new Error("The computed path is missing a span endpoint.");
        const previous = segment[segment.length - 1];
        if (!previous || point.some((v, i) => v !== previous[i]))
          segment.push(point);
      } while (!sameXY(point, span.end, span));
    }
    if (cursor !== points.length)
      throw new Error("The computed path contains unexpected points.");
    return paths;
  }

  function writeGcode(result, settings) {
    const { coordinates, offsets, bounds } = result;
    if (
      !coordinates.length ||
      coordinates.length % 3 ||
      offsets.length < 2 ||
      offsets[0] !== 0 ||
      offsets[offsets.length - 1] !== coordinates.length / 3
    )
      throw new Error("Generate a nonempty toolpath before exporting G-code.");
    if (
      !Number.isInteger(settings.decimals) ||
      settings.decimals < 3 ||
      settings.decimals > 6
    )
      throw new Error("Choose between 3 and 6 decimal places.");
    const decimal = (value) => {
      if (!Number.isFinite(value) || Math.abs(value) >= 1e21)
        throw new Error(
          "G-code coordinates and settings must be finite decimal values.",
        );
      const text = value.toFixed(settings.decimals);
      return Number(text) === 0 ? (0).toFixed(settings.decimals) : text;
    };
    for (const key of ["feed", "plunge"]) {
      if (!(Number(decimal(settings[key])) > 0))
        throw new Error(
          "Cutting and plunge feeds must be positive at the selected precision.",
        );
    }
    if (!/^G5[4-9]$/.test(settings.workOffset))
      throw new Error("Choose a work coordinate system from G54 to G59.");
    if (
      !["off", "cw", "ccw"].includes(settings.spindle) ||
      !["off", "flood"].includes(settings.coolant)
    )
      throw new Error("Choose a supported spindle and coolant setting.");
    if (
      settings.spindle !== "off" &&
      (!Number.isSafeInteger(settings.rpm) || settings.rpm <= 0)
    )
      throw new Error("Spindle speed must be a positive whole number.");
    if (!Number.isFinite(settings.dwell) || settings.dwell < 0)
      throw new Error("Spindle delay must be finite and nonnegative.");
    let top = bounds.max[2];
    for (let i = 0; i < coordinates.length; i++) {
      decimal(coordinates[i]);
      if (i % 3 === 2) top = Math.max(top, coordinates[i]);
    }
    const retract = decimal(settings.retract);
    if (settings.retract <= top || Number(retract) <= Number(decimal(top)))
      throw new Error(
        `Retract Z must be above the model and toolpath (highest Z ${decimal(top)} mm), including after rounding.`,
      );
    const comment = (value) =>
      String(value)
        .replace(/[^a-zA-Z0-9 ._:+-]/g, " ")
        .slice(0, 100);
    const lines = [
      "(OpenCAMLib surface finishing - verify setup before machining)",
      `(Model: ${comment(result.model)})`,
      `(Algorithm: ${comment(result.params.algorithm)})`,
      `(Cutter: ${comment(result.params.cutter)} D${decimal(result.params.diameter)})`,
      "(Millimeters, absolute model coordinates, tool-tip positions)",
      "G21",
      "G90",
      "G17",
      "G94",
      "G40",
      "G80",
      settings.workOffset,
      "M9",
      `G0 Z${retract}`,
    ];
    if (settings.spindle !== "off") {
      lines.push(`${settings.spindle === "cw" ? "M3" : "M4"} S${settings.rpm}`);
      if (settings.dwell > 0) lines.push(`G4 P${decimal(settings.dwell)}`);
    }
    if (settings.coolant === "flood") lines.push("M8");
    let paths = 0;
    for (let n = 0; n < offsets.length - 1; n++) {
      const start = offsets[n],
        end = offsets[n + 1];
      if (
        !Number.isSafeInteger(start) ||
        !Number.isSafeInteger(end) ||
        start < 0 ||
        end < start ||
        end > coordinates.length / 3
      )
        throw new Error("Invalid toolpath segment boundaries.");
      if (start === end) continue;
      paths++;
      lines.push(
        `(Path ${paths})`,
        `G0 X${decimal(coordinates[start * 3])} Y${decimal(coordinates[start * 3 + 1])}`,
        `G1 Z${decimal(coordinates[start * 3 + 2])} F${decimal(settings.plunge)}`,
      );
      let feed = settings.plunge;
      for (let i = start + 1; i < end; i++) {
        const x = decimal(coordinates[i * 3]),
          y = decimal(coordinates[i * 3 + 1]),
          z = decimal(coordinates[i * 3 + 2]);
        const verticalDown =
          x === decimal(coordinates[(i - 1) * 3]) &&
          y === decimal(coordinates[(i - 1) * 3 + 1]) &&
          Number(z) < Number(decimal(coordinates[(i - 1) * 3 + 2]));
        const nextFeed = verticalDown ? settings.plunge : settings.feed;
        lines.push(
          `G1 X${x} Y${y} Z${z}${nextFeed !== feed ? ` F${decimal(nextFeed)}` : ""}`,
        );
        feed = nextFeed;
      }
      lines.push(`G0 Z${retract}`);
    }
    lines.push("M9");
    if (settings.spindle !== "off") lines.push("M5");
    lines.push("M2");
    return { text: lines.join("\n") + "\n", lines: lines.length, paths };
  }

  return {
    fitCircularBounds,
    validateCircularSettings,
    planPaths,
    splitPath,
    writeGcode,
  };
})();
