import Operation from './operation'
import ocl from './ocl'

class NodeJSWaterline extends Operation {
    constructor() {
        super()
        this.actualClass = new ocl.Waterline()
    }

    setZ(z: number) {
        this.actualClass.setZ(z)
    }

    run() {
        this.actualClass.run()
        return Promise.resolve(this.actualClass.getLoops())
    }
}

export default NodeJSWaterline