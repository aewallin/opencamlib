"""Test bundled pthread bindings in headless Chrome with real isolation headers.

Usage: python3 tests/wasm_browser.py build/wasm-threads/ocl.js src/npmpackage/browser.js
Set CHROME_BIN or pass --chrome when Chrome is not on PATH.
"""
import argparse
import http.server
import json
import os
from pathlib import Path
import shutil
import subprocess
import tempfile
import threading
import time


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("module", type=Path)
    parser.add_argument("bundle", type=Path)
    parser.add_argument("--chrome", default=os.environ.get("CHROME_BIN"))
    args = parser.parse_args()
    chrome = args.chrome or shutil.which("google-chrome")
    if not chrome:
        mac_chrome = Path("/Applications/Google Chrome.app/Contents/MacOS/Google Chrome")
        if mac_chrome.exists():
            chrome = str(mac_chrome)
    chrome = chrome or shutil.which("chromium")
    if not chrome:
        parser.error("Chrome not found; pass --chrome or set CHROME_BIN")
    assets = {
        "/ocl.js": args.module.read_bytes(),
        "/bundle.js": args.bundle.read_bytes(),
        "/tests.js": Path(__file__).with_name("wasm_threading.js").read_bytes(),
        "/worker.js": b"""
importScripts('/bundle.js', '/tests.js');
checkThreadedModule().then(result => postMessage({result}), error => postMessage({error: String(error.stack || error)}));
""",
        "/": b"""<!doctype html><meta charset="utf-8"><title>WASM pthread tests</title>
<script>
const report = result => fetch('/result', {method: 'POST', body: JSON.stringify(result)});
addEventListener('error', event => report({error: event.message}));
addEventListener('unhandledrejection', event => report({error: String(event.reason)}));
</script><script src="/bundle.js"></script><script src="/tests.js"></script><script>
(async () => {
    const page = await checkThreadedModule();
    const worker = await new Promise((resolve, reject) => {
        const worker = new Worker('/worker.js');
        worker.onerror = event => reject(new Error(event.message));
        worker.onmessage = event => {
            worker.terminate();
            if (event.data.error) reject(new Error(event.data.error));
            else resolve(event.data.result);
        };
    });
    await report({page, worker});
})().catch(error => report({error: String(error.stack || error)}));
</script>""",
    }
    completed = threading.Event()
    result = []

    class Handler(http.server.BaseHTTPRequestHandler):
        def log_message(self, *_):
            pass

        def do_GET(self):
            body = assets.get(self.path)
            if body is None:
                self.send_error(404)
                return
            self.send_response(200)
            self.send_header("Cross-Origin-Opener-Policy", "same-origin")
            self.send_header("Cross-Origin-Embedder-Policy", "require-corp")
            self.send_header("Content-Type", "text/html; charset=utf-8" if self.path == "/" else "application/javascript; charset=utf-8")
            self.send_header("Content-Length", str(len(body)))
            self.end_headers()
            self.wfile.write(body)

        def do_POST(self):
            if self.path != "/result":
                self.send_error(404)
                return
            result.append(json.loads(self.rfile.read(int(self.headers["Content-Length"]))))
            self.send_response(204)
            self.end_headers()
            completed.set()

    with http.server.ThreadingHTTPServer(("127.0.0.1", 0), Handler) as server, tempfile.TemporaryDirectory(prefix="ocl-chrome-") as profile:
        server_thread = threading.Thread(target=server.serve_forever, daemon=True)
        server_thread.start()
        with tempfile.TemporaryFile() as log:
            process = subprocess.Popen([chrome, "--headless", "--disable-gpu", "--no-first-run",
                "--no-default-browser-check", "--disable-dev-shm-usage",
                f"--user-data-dir={profile}", f"http://127.0.0.1:{server.server_port}/"], stdout=log, stderr=log)
            try:
                deadline = time.monotonic() + 90
                while not completed.wait(1):
                    if process.poll() is not None:
                        raise RuntimeError(f"Chrome exited before reporting results ({process.returncode})")
                    if time.monotonic() >= deadline:
                        raise RuntimeError("Browser pthread test timed out")
                if "error" in result[0]:
                    raise RuntimeError(result[0]["error"])
                if set(result[0]) != {"page", "worker"}:
                    raise RuntimeError(f"Incomplete browser result: {result[0]}")
                print(json.dumps(result[0]))
            except Exception:
                log.seek(0)
                print(log.read().decode(errors="replace"))
                raise
            finally:
                process.terminate()
                try:
                    process.wait(timeout=10)
                except subprocess.TimeoutExpired:
                    process.kill()
                    process.wait()
                server.shutdown()
                server_thread.join()


if __name__ == "__main__":
    main()
