import ocl from './ocl'

import Operation from './operation'
import Path from './path'

class AdaptivePathDropCutter extends Operation {
    protected minSampling?: number
    protected path?: Path

    setMinSampling(minSampling: number) {
        this.minSampling = minSampling
    }

    getCLPoints() {
        if (!this.actualClass) {
            throw new Error('Call run() before getCLPoints()')
        }
        return this.actualClass.getCLPoints()
    }

    setPath(path: Path) {
        this.path = path
    }

    run() {
        this.actualClass = new ocl.AdaptivePathDropCutter()
        if (!this.surface) {
            throw new Error('Please set a STLSurface using setSTL()')
        }
        if (!this.cutter) {
            throw new Error('Please set a MillingCutter using setCutter()')
        }
        if (!this.path) {
            throw new Error('Please set a Path using setPath()')
        }
        this.actualClass.setSTL(this.surface.actualClass)
        this.setCutterOnActualClass()
        this.actualClass.setPath(this.path.actualClass)
        this.actualClass.setSampling(this.sampling)
        this.actualClass.setMinSampling(this.minSampling)
        this.actualClass.run()
    }
}

export default AdaptivePathDropCutter