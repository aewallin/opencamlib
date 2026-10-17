import STLSurf from '../stlsurf'
import MillingCutter from '../millingcutter'
import Path from '../path'
import { buildPath, buildSurface, release } from '../native'
import initialize from './ocl'

export type Point3 = [number, number, number]

class Operation {
    protected actualClass: any
    protected module: any
    private readonly ready: Promise<void>
    private disposal?: Promise<void>
    private surface: any
    private cutter: any
    private path: any

    constructor(name: string) {
        this.ready = initialize().then((ocl: any) => {
            this.module = ocl
            this.actualClass = new ocl[name]()
        })
    }

    protected whenReady<T>(action: () => T): Promise<T> {
        if (this.disposal) return Promise.reject(new Error('Operation has been disposed'))
        // These callbacks do synchronous WASM work. Promise reactions preserve
        // call order, so no separate queue is needed after initialization.
        return this.ready.then(action)
    }

    private replaceInput(key: 'surface' | 'cutter' | 'path',
                         create: () => any, attach: (input: any) => void) {
        return this.whenReady(() => {
            const native = create()
            try { attach(native) }
            catch (error) { release(native); throw error }
            release(this[key])
            this[key] = native
        })
    }

    setSTL(surface: STLSurf) {
        return this.replaceInput('surface', () => buildSurface(this.module, surface),
            native => this.actualClass.setSTL(native))
    }

    setCutter(cutter: MillingCutter) {
        return this.replaceInput('cutter', () => {
            const [kind, args] = cutter.serialize()
            return new this.module[kind](...args)
        }, native => this.actualClass.setCutter(native))
    }

    protected setNativePath(path: Path) {
        return this.replaceInput('path', () => buildPath(this.module, path),
            native => this.actualClass.setPath(native))
    }

    setSampling(sampling: number) {
        return this.whenReady(() => { this.actualClass.setSampling(sampling) })
    }

    setThreads(threads: number) {
        return this.whenReady(() => {
            if (!Number.isInteger(threads) || threads < 0 || threads > 2147483647)
                throw new Error('Thread count must be a nonnegative 32-bit integer')
            this.actualClass.setThreads(threads)
        })
    }

    getThreads(): Promise<number> {
        return this.whenReady(() => this.actualClass.getThreads())
    }

    setZ(z: number) {
        return this.whenReady(() => { this.actualClass.setZ(z) })
    }

    run() {
        return this.whenReady(() => { this.actualClass.run() })
    }

    dispose(): Promise<void> {
        if (this.disposal) return this.disposal
        const cleanup = () => {
            release(this.actualClass)
            release(this.path)
            release(this.cutter)
            release(this.surface)
            this.actualClass = this.path = this.cutter = this.surface = undefined
        }
        // Runs after already registered calls, even when startup fails.
        this.disposal = this.ready.then(cleanup, cleanup)
        return this.disposal
    }

    protected pointsToArray(points: any): Point3[] {
        const result: Point3[] = []
        try {
            for (let i = 0; i < points.size(); i++) {
                const point = points.get(i)
                try { result.push([point.x, point.y, point.z]) }
                finally { release(point) }
            }
        } finally { release(points) }
        return result
    }

    protected loopsToArray(loops: any): Point3[][] {
        const result: Point3[][] = []
        try {
            for (let i = 0; i < loops.size(); i++) result.push(this.pointsToArray(loops.get(i)))
        } finally { release(loops) }
        return result
    }
}
export default Operation
