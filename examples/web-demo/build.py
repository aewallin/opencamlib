#!/usr/bin/env python3
"""Package the Tux model and two SINGLE_FILE Emscripten builds into one HTML file."""
import argparse
import base64
import gzip
from pathlib import Path
import re
import struct

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[1]


def binary_stl(path):
    data = path.read_bytes()
    if len(data) >= 84 and len(data) == 84 + 50 * struct.unpack_from('<I', data, 80)[0]:
        return data
    numbers = rb'([-+\d.eE]+)\s+([-+\d.eE]+)\s+([-+\d.eE]+)'
    vertices = [tuple(map(float, match)) for match in re.findall(rb'\bvertex\s+' + numbers, data)]
    if not vertices or len(vertices) % 3:
        raise ValueError('Expected a triangle STL')
    output = bytearray(b'OpenCAMLib Tux demo'.ljust(80, b'\0'))
    output += struct.pack('<I', len(vertices) // 3)
    for i in range(0, len(vertices), 3):
        output += struct.pack('<12fH', 0, 0, 0, *vertices[i], *vertices[i+1], *vertices[i+2], 0)
    return bytes(output)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--serial', type=Path, default=ROOT / 'build/web-demo-serial/ocl.js')
    parser.add_argument('--threaded', type=Path, default=ROOT / 'build/web-demo-threaded/ocl.js')
    parser.add_argument('--stl', type=Path, default=ROOT / 'stl/gnu_tux_mod.stl')
    parser.add_argument('--output', type=Path, default=HERE / 'index.html')
    args = parser.parse_args()
    for path in (args.serial, args.threaded, args.stl):
        if not path.is_file():
            parser.error(f'Missing input: {path}. See examples/web-demo/README.rst for build instructions.')
    html = (HERE / 'demo.html.in').read_text(encoding='utf-8')
    for name, data in [('STL', binary_stl(args.stl)), ('SERIAL', args.serial.read_bytes()),
                       ('THREADED', args.threaded.read_bytes())]:
        html = html.replace(f'@@{name}@@', base64.b64encode(gzip.compress(data, mtime=0)).decode())
    if re.search(r'@@[A-Z]+@@', html):
        raise ValueError('Unresolved template placeholder')
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(html, encoding='utf-8')
    print(f'{args.output}: {len(html.encode()) / 1024:.0f} KiB; model and both WASM builds embedded')


if __name__ == '__main__':
    main()
