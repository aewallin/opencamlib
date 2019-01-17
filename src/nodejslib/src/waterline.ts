import ocl from './ocl'

import Operation from './operation';

class Waterline extends Operation {
    protected z?: number

    setZ(z: number) {
        this.z = z
    }

    getLoops() {
        if (!this.actualClass) {
            throw new Error('Call run() before getLoops()')
        }
        return this.actualClass.getLoops()
    }

    run() {
        this.actualClass = new ocl.Waterline()
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
        this.actualClass.run()
    }
}

export default Waterline