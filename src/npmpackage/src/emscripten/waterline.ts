import Operation from './operation';
import ocl from './ocl'

class EmscriptenWaterline extends Operation {
    constructor() {
        super()
        this.chain = this.chain.then(() => {
            this.actualClass = new ocl.Waterline()
        })
    }

    setZ(z: number) {
        this.chain = this.chain.then(() => {
            this.actualClass.setZ(z)
        })
    }

    run() {
        this.actualClass.run()
    }

    getLoops() {
        return this.loopsToArray(this.actualClass.getLoops())
    }
}

export default EmscriptenWaterline