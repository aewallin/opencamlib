// Run against the public Node or Emscripten TypeScript wrappers after compilation.
const assert = require('assert/strict')
const fs = require('fs')
const path = require('path')

// Calls made before startup must execute in order, including disposal. Also
// exercise the same sequence after startup, when the ready promise is settled.
async function checkCallOrdering(ocl, startRuntime) {
    const operation = new ocl.PathDropCutter()
    if (!startRuntime) await operation.setThreads(1)
    const toolpath = new ocl.Path()
    toolpath.append(new ocl.Line(new ocl.Point(0,0,-10), new ocl.Point(1,0,-10)))
    const pending = [
        operation.setSTL(new ocl.STLSurf()),
        operation.setCutter(new ocl.CylCutter(.5,5)),
        operation.setPath(toolpath),
        assert.rejects(operation.setSampling(0), /Sampling/),
        operation.setSampling(.2),
        operation.setZ(2),
        operation.run()
    ]
    const first = operation.getCLPoints()
    pending.push(operation.setZ(3), operation.run())
    const second = operation.getCLPoints()
    const disposal = operation.dispose()
    assert.equal(operation.dispose(), disposal)
    await assert.rejects(operation.run(), /disposed/)
    if (startRuntime) {
        let finished = false
        disposal.then(() => { finished = true })
        await Promise.resolve()
        assert.equal(finished, false, 'disposal must wait for runtime startup')
        startRuntime()
    }
    await Promise.all([...pending, disposal])
    const pointsAtTwo = await first
    const pointsAtThree = await second
    assert(pointsAtTwo.length > 1)
    assert.equal(pointsAtTwo.length, pointsAtThree.length)
    assert(pointsAtTwo.every(p => p[2] === 2))
    assert(pointsAtThree.every(p => p[2] === 3))
}

async function main() {
    const backend = process.argv[2] || 'napi'
    let startRuntime, initialization, runtime
    // Inject a Node-capable WASM build without changing the browser package loader.
    if (backend === 'emscripten') {
        const loader = require.resolve('../src/npmpackage/lib/emscripten/ocl.js')
        const factory = require(path.resolve(process.argv[3]))
        runtime = new Promise(resolve => { startRuntime = () => resolve(factory()) })
        initialization = runtime
        require.cache[loader] = { exports: { __esModule: true, default: () => initialization }, loaded: true }
    }
    const ocl = require(`../src/npmpackage/lib/${backend}/index.js`)
    if (backend === 'emscripten') {
        await checkCallOrdering(ocl, startRuntime)
        await checkCallOrdering(ocl)
        const startupError = new Error('WASM startup failed')
        initialization = Promise.reject(startupError)
        const failed = new ocl.PathDropCutter()
        try {
            // A failed first call must not hide the startup error on later calls.
            await assert.rejects(failed.run(), error => error === startupError)
            await assert.rejects(failed.run(), error => error === startupError)
        } finally {
            await failed.dispose()
            await failed.dispose()
            initialization = runtime
        }
    }
    const fixture = fs.readFileSync(path.join(__dirname, 'fixtures/path_contract.txt'), 'utf8')
    for (const row of fixture.trim().split('\n')) {
        const [scale, floor, expected] = row.split(/\s+/).map(Number)
        for (const Class of [ocl.PathDropCutter, ocl.AdaptivePathDropCutter]) {
            const operation = new Class()
            try {
                const surface = new ocl.STLSurf()
                surface.addTriangle(new ocl.Triangle(new ocl.Point(-10*scale,-10*scale,scale),
                    new ocl.Point(10*scale,-10*scale,scale), new ocl.Point(0,10*scale,scale)))
                await operation.setSTL(surface)
                await operation.setCutter(new ocl.CylCutter(.5*scale,5*scale))
                const toolpath = new ocl.Path()
                toolpath.append(new ocl.Line(new ocl.Point(-scale,0,-10*scale), new ocl.Point(scale,0,-10*scale)))
                toolpath.append(new ocl.Arc(new ocl.Point(scale,0,-10*scale), new ocl.Point(-scale,0,-10*scale),
                    new ocl.Point(0,0,-10*scale), true))
                await operation.setPath(toolpath)
                if (global.gc) global.gc()
                await operation.setSampling(.2*scale)
                await operation.setZ(floor)
                if (backend === 'emscripten') {
                    await operation.setThreads(1)
                    assert.equal(await operation.getThreads(), 1)
                    await assert.rejects(operation.setThreads(-1), /Thread count/)
                    await assert.rejects(operation.setThreads(1.5), /Thread count/)
                }
                await operation.run()
                const points = await operation.getCLPoints()
                assert(points.length > 5)
                assert(points.every(p => Math.abs(p[2]-expected) < 1e-9*scale))
                if (backend === 'emscripten') await operation.setThreads(4)
                await operation.run()
                assert.deepEqual(await operation.getPoints(), points)
                console.log(JSON.stringify(points))
                await assert.rejects(async () => operation.setSampling(0), /Sampling/)
            } finally {
                await operation.dispose()
                await operation.dispose()
            }
            await assert.rejects(async () => operation.run(), /disposed/)
        }
    }
    if (backend === 'napi') {
        const native = require(process.env.OCL_NODE_PATH)
        assert.throws(() => new native.Point(1))
        assert.throws(() => new native.Line({}, {}))
        const operation = new native.PathDropCutter()
        assert.throws(() => operation.run())
        assert.throws(() => operation.setSTL({}))
        operation.dispose()
    }
}
main().catch(error => { console.error(error); process.exitCode = 1 })
