import Operation from './operation';
import opencamlib from './ocl'

class EmscriptenWaterline extends Operation {
    constructor() {
        super()
        this.chain = this.chain.then(() => {
            return opencamlib.then((ocl: any) => {
                this.actualClass = new ocl.Waterline()
            })
        })
    }

    setZ(z: number) {
        this.chain = this.chain.then(() => {
            this.actualClass.setZ(z)
        })
    }

    run() {
        this.chain = this.chain.then(() => {
            this.actualClass.run()
        })
    }

    getLoops() {
        return this.chain.then(() => {
            return this.loopsToArray(this.actualClass.getLoops())
        })
    }
}

export default EmscriptenWaterline