import ocl from './ocl'

import Operation from './operation';

class AdaptiveWaterline extends Operation {
    protected z?: number
    protected minSampling?: number

    setZ(z: number) {
        this.z = z
    }

    setMinSampling(minSampling: number) {
        this.minSampling = minSampling
    }

    getLoops() {
        if (!this.actualClass) {
            throw new Error('Call run() before getLoops()')
        }
        return this.actualClass.getLoops()
    }

    run() {
        this.actualClass = new ocl.AdaptiveWaterline()
        if (!this.surface) {
            throw new Error('Please set a STLSurface using setSTL()')
        }
        if (!this.cutter) {
            throw new Error('Please set a MillingCutter using setCutter()')
        }
        this.actualClass.setSTL(this.surface.actualClass)
        this.setCutterOnActualClass()
        this.actualClass.setZ(this.z)
        this.actualClass.setSampling(this.sampling)
        this.actualClass.setMinSampling(this.minSampling)
        this.actualClass.run()
    }
}

export default AdaptiveWaterline