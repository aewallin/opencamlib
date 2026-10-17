"""Run the shared path contract against an installed or locally built Python module."""
import gc
import json
from pathlib import Path
import sys

if len(sys.argv) > 1:
    sys.path.insert(0, sys.argv[1])
    import ocl
else:
    from opencamlib import ocl

for row in (Path(__file__).parent / "fixtures/path_contract.txt").read_text().splitlines():
    scale, floor, expected = map(float, row.split())
    for cls in (ocl.PathDropCutter, ocl.AdaptivePathDropCutter):
        operation = cls()
        surface = ocl.STLSurf()
        surface.addTriangle(ocl.Triangle(ocl.Point(-10*scale, -10*scale, scale),
                            ocl.Point(10*scale, -10*scale, scale), ocl.Point(0, 10*scale, scale)))
        operation.setSTL(surface)
        operation.setCutter(ocl.CylCutter(.5*scale, 5*scale))
        path = ocl.Path()
        path.append(ocl.Line(ocl.Point(-scale, 0, -10*scale), ocl.Point(scale, 0, -10*scale)))
        path.append(ocl.Arc(ocl.Point(scale, 0, -10*scale), ocl.Point(-scale, 0, -10*scale),
                            ocl.Point(0, 0, -10*scale), True))
        operation.setPath(ocl.Path(path))  # Exercise copying and keep_alive together.
        del path, surface
        gc.collect()
        operation.setSampling(.2*scale)
        operation.setZ(floor)
        operation.setThreads(1)
        operation.run()
        points = [[p.x, p.y, p.z] for p in operation.getCLPoints()]
        assert len(points) > 5
        assert all(abs(p[2]-expected) < 1e-9*scale for p in points)
        operation.run()
        assert points == [[p.x, p.y, p.z] for p in operation.getPoints()]
        print(json.dumps(points))
        try:
            operation.setSampling(0)
        except ValueError:
            pass
        else:
            raise AssertionError("Invalid sampling accepted")
