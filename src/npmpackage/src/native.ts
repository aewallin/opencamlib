import Path from './path'
import STLSurf from './stlsurf'

// Embind objects need explicit deletion; Node wrappers are owned by the GC.
export function release(value: any) { value?.delete?.() }

export function buildSurface(ocl: any, surface: STLSurf): any {
    const native = new ocl.STLSurf()
    try {
        for (const vertices of surface.triangles) {
            const points: any[] = []
            let triangle: any
            try {
                for (const vertex of vertices) points.push(new ocl.Point(...vertex))
                triangle = new ocl.Triangle(...points)
                native.addTriangle(triangle)
            } finally {
                release(triangle)
                points.forEach(release)
            }
        }
        return native
    } catch (error) { release(native); throw error }
}

export function buildPath(ocl: any, path: Path): any {
    const native = new ocl.Path()
    try {
        for (const [kind, args] of path.serialize()) {
            if (kind !== 'Line' && kind !== 'Arc') throw new Error(`Unsupported span: ${kind}`)
            const points: any[] = []
            let span: any
            try {
                for (const point of args.slice(0, kind === 'Arc' ? 3 : 2))
                    points.push(new ocl.Point(point.x, point.y, point.z))
                span = kind === 'Arc' ? new ocl.Arc(...points, args[3]) : new ocl.Line(...points)
                if (native.append) native.append(span)
                else if (kind === 'Arc') native.appendArc(span)
                else native.appendLine(span)
            } finally {
                release(span)
                points.forEach(release)
            }
        }
        return native
    } catch (error) { release(native); throw error }
}
