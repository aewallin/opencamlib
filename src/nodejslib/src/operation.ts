import ocl from './ocl'

import STLSurf from './stlsurf'
import CylCutter from './cylcutter'
import BallCutter from './ballcutter'
import BullCutter from './bullcutter'
import ConeCutter from './conecutter'

type Cutter = CylCutter | BallCutter | BullCutter | ConeCutter

class Operation {
    protected surface?: STLSurf
    protected cutter?: Cutter
    protected sampling?: number
    public actualClass: any

    setSTL(surface: STLSurf) {
        this.surface = surface
    }

    setCutter(cutter: Cutter) {
        this.cutter = cutter
    }

    setSampling(sampling: number) {
        this.sampling = sampling
    }

    protected setCutterOnActualClass() {
        if (!this.cutter) return
        if (this.cutter instanceof CylCutter) {
            this.actualClass.setCylCutter(this.cutter.actualClass)
        } else if (this.cutter instanceof BallCutter) {
            this.actualClass.setBallCutter(this.cutter.actualClass)
        } else if (this.cutter instanceof BullCutter) {
            this.actualClass.setBullCutter(this.cutter.actualClass)
        } else if (this.cutter instanceof ConeCutter) {
            this.actualClass.setConeCutter(this.cutter.actualClass)
        }
    }
}

export default Operation