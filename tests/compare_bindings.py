"""Compare complete ordered coordinates emitted by the shared binding fixture."""
import json
import math
from pathlib import Path
import sys

def read(filename):
    return [json.loads(line) for line in Path(filename).read_text().splitlines()]

reference = read(sys.argv[1])
assert len(reference) == 12
for filename in sys.argv[2:]:
    actual = read(filename)
    assert len(actual) == len(reference), filename
    for expected_points, actual_points in zip(reference, actual):
        assert len(actual_points) == len(expected_points), filename
        for expected, actual in zip(expected_points, actual_points):
            assert len(actual) == 3, filename
            assert all(math.isclose(a, b, rel_tol=1e-10, abs_tol=1e-10)
                       for a, b in zip(actual, expected)), (filename, expected, actual)
    print(f"{filename}: all 12 fixtures match")
