import Operation from './operation'
import Path from '../path'
import ocl from './ocl'

class NapiAdaptivePathDropCutter extends Operation {
    constructor() {
        super()
        this.actualClass = new ocl.AdaptivePathDropCutter()
    }

    setMinSampling(minSampling: number) {
        this.actualClass.setMinSampling(minSampling)
    }

    setPath(path: Path) {
        const actualPath = new ocl.Path()
        path.serialize().forEach(span => {
            if (span[0] === 'Line') {
                const p1 = new ocl.Point(span[1][0].x, span[1][0].y, span[1][0].z)
                const p2 = new ocl.Point(span[1][1].x, span[1][1].y, span[1][1].z)
                const l = new ocl.Line(p1, p2)
                actualPath.append(l)
            }
        })
        this.actualClass.setPath(actualPath)
    }

    setZ(z: number) {
        this.actualClass.setZ(z)
    }

    run() {
        this.actualClass.run()
    }

    getCLPoints() {
        return this.actualClass.getCLPoints()
    }
}

export default NapiAdaptivePathDropCutter