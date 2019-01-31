import Operation from './operation'
import ocl from './ocl'

class NodeJSAdaptiveWaterline extends Operation {
    constructor() {
        super()
        this.actualClass = new ocl.AdaptiveWaterline()
    }

    setZ(z: number) {
        this.actualClass.setZ(z)
    }

    setMinSampling(minSampling: number) {
        this.actualClass.setMinSampling(minSampling)
    }

    run() {
        this.actualClass.run()
        return Promise.resolve(this.actualClass.getLoops())
    }
}

export default NodeJSAdaptiveWaterline