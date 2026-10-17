import { buildSurface } from '../native'
import STLSurf from '../stlsurf'
import MillingCutter from '../millingcutter'
import ocl from './ocl'
import CylCutter from '../cylcutter'
import BallCutter from '../ballcutter'
import BullCutter from '../bullcutter'
import ConeCutter from '../conecutter'

class Operation {
    protected actualClass: any

    setSTL(surface: STLSurf) {
        this.actualClass.setSTL(buildSurface(ocl, surface))
    }

    dispose() { this.actualClass.dispose() }

    setCutter(cutter: MillingCutter) {
        const cutterDef = cutter.serialize()
        const [cutterType, cutterArgs] = cutterDef
        const Cutter = ocl[cutterType]
        const actualCutter = new Cutter(...cutterArgs)
        if (cutter instanceof CylCutter) {
            this.actualClass.setCylCutter(actualCutter)
        } else if (cutter instanceof BallCutter) {
            this.actualClass.setBallCutter(actualCutter)
        } else if (cutter instanceof BullCutter) {
            this.actualClass.setBullCutter(actualCutter)
        } else if (cutter instanceof ConeCutter) {
            this.actualClass.setConeCutter(actualCutter)
        }
    }

    setSampling(sampling: number) {
        this.actualClass.setSampling(sampling)
    }
}

export default Operation