// Loaded by the browser test on both the page and a dedicated Web Worker.
async function checkThreadedModule() {
    const module = await ocl.initialize({ mainScriptUrlOrBlob: new URL('/ocl.js', location.href).href })
    const check = (condition, message) => { if (!condition) throw new Error(message) }
    check(crossOriginIsolated, 'Test server must enable cross-origin isolation')
    check(module.maxThreads() > 1, 'Expected a pthread-enabled build')
    const surface = new ocl.STLSurf()
    const corners = [[-4, -4, 0], [4, -4, 0], [4, 4, 0], [-4, 4, 0]]
    for (let i = 0; i < 4; ++i)
        surface.addTriangle(new ocl.Triangle(new ocl.Point(...corners[i]),
            new ocl.Point(...corners[(i + 1) % 4]), new ocl.Point(0, 0, 3)))
    const path = new ocl.Path()
    for (let i = 0; i < 16; ++i)
        path.append(new ocl.Line(new ocl.Point(-5, -4 + i * .5, -1), new ocl.Point(5, -4 + i * .5, -1)))
    path.append(new ocl.Arc(new ocl.Point(2, 0, -1), new ocl.Point(-2, 0, -1), new ocl.Point(0, 0, -1), true))
    const results = []
    for (const name of ['PathDropCutter', 'AdaptivePathDropCutter', 'Waterline', 'AdaptiveWaterline']) {
        const operation = new ocl[name]()
        try {
            await operation.setSTL(surface)
            await operation.setCutter(new ocl.BallCutter(.6, 5))
            await operation.setSampling(.1)
            const isPath = name.includes('Path')
            if (isPath) await operation.setPath(path)
            await operation.setZ(isPath ? -1 : 1)
            await operation.setThreads(1)
            await operation.run()
            const output = () => isPath ? operation.getCLPoints() : operation.getLoops()
            const serial = await output()
            check(serial.length > 0, name + ': empty result')
            for (const threads of [4, 1000, 0, 2, 4]) {
                await operation.setThreads(threads)
                check(await operation.getThreads() === Math.min(Math.max(1, threads), module.maxThreads()),
                    name + ': thread count was not bounded to the pool')
                await operation.run()
                check(JSON.stringify(await output()) === JSON.stringify(serial), name + ': threaded result differs')
            }
            results.push({ operation: name, outputs: serial.length })
        } finally {
            await operation.dispose()
        }
    }
    return { threads: module.maxThreads(), results }
}
