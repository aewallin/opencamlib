import ocl from './ocl'

import MillingCutter from './millingcutter'

class BullCutter extends MillingCutter {
    actualClass: any

    constructor(public d: number, public r: number, public l: number) {
        super()
        this.actualClass = new ocl.BullCutter(d, r, l)
    }
}

export default BullCutter