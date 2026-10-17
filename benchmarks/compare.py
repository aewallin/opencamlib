#!/usr/bin/env python3
"""Alternate two cutter_benchmark executables and report median timings."""
import argparse
import json
import math
import os
from pathlib import Path
import statistics
import subprocess


def run(binary, model, mode, threads):
    result = subprocess.run(
        [str(binary.resolve()), str(model), mode, str(threads), "1"],
        env=dict(os.environ, OMP_NUM_THREADS=str(threads)),
        check=True, capture_output=True, text=True,
    )
    return json.loads(result.stdout.splitlines()[-1])


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("before", type=Path)
    parser.add_argument("after", type=Path)
    parser.add_argument("models", nargs="+", type=Path)
    parser.add_argument("--threads", nargs="+", type=int, default=[1, 4])
    parser.add_argument("--repetitions", type=int, default=5)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    if args.repetitions < 1 or any(t < 1 for t in args.threads):
        parser.error("threads and repetitions must be positive")
    report = {"before": str(args.before), "after": str(args.after), "results": []}
    for model in args.models:
        for mode in ["waterline", "adaptive-waterline", "dropcutter", "adaptive-dropcutter"]:
            for threads in args.threads:
                samples = {"before": [], "after": []}
                for iteration in range(args.repetitions):
                    order = [("before", args.before), ("after", args.after)]
                    if iteration % 2: order.reverse()
                    for name, binary in order:
                        samples[name].append(run(binary, model, mode, threads))
                reference = samples["before"][0]
                for result in samples["before"] + samples["after"]:
                    if (result["points"] != reference["points"]
                            or result["triangles"] != reference["triangles"]
                            or not math.isclose(result["checksum"], reference["checksum"], rel_tol=1e-12, abs_tol=1e-12)):
                        raise RuntimeError(f"Workload changed: {model} {mode} {threads}")
                medians = {
                    name: {field: statistics.median(r[field] for r in values)
                           for field in ["setup_ms", "run_ms", "total_ms"]}
                    for name, values in samples.items()
                }
                row = dict(model=str(model), mode=mode, threads=threads,
                           medians=medians, samples=samples,
                           speedup=medians["before"]["total_ms"] / medians["after"]["total_ms"])
                report["results"].append(row)
                args.output.write_text(json.dumps(report, indent=2) + "\n")
                print(f"{model.name} {mode} threads={threads}: "
                      f'{medians["before"]["total_ms"]:.2f} -> '
                      f'{medians["after"]["total_ms"]:.2f} ms ({row["speedup"]:.2f}x)', flush=True)


if __name__ == "__main__":
    main()
