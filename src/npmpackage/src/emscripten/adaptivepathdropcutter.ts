import Path from '../path'
import Operation from './operation'
import ocl from './ocl'

class EmscriptenAdaptivePathDropCutter extends Operation {
    constructor() {
        super()
        this.chain = this.chain.then(() => {
            this.actualClass = new ocl.AdaptivePathDropCutter()
        })
    }

    setMinSampling(minSampling: number) {
        this.chain = this.chain.then(() => {
            this.actualClass.setMinSampling(minSampling)
        })
    }

    setPath(path: Path) {
        this.chain = this.chain.then(() => {
            const actualPath = new ocl.Path()
            path.serialize().forEach(span => {
                if (span[0] === 'Line') {
                    const p1 = new ocl.Point(span[1][0].x, span[1][0].y, span[1][0].z)
                    const p2 = new ocl.Point(span[1][1].x, span[1][1].y, span[1][1].z)
                    const l = new ocl.Line(p1, p2)
                    actualPath.appendLine(l)
                }
            })
            this.actualClass.setPath(actualPath)
        })
    }

    run() {
        return this.chain.then(() => {
            this.actualClass.run()
            return Promise.resolve(this.pointsToArray(this.actualClass.getCLPoints()))
        })
    }
}

export default EmscriptenAdaptivePathDropCutter