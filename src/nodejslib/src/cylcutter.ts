import ocl from './ocl'

import MillingCutter from './millingcutter'

class CylCutter extends MillingCutter {
    actualClass: any

    constructor(public d: number, public l: number) {
        super()
        this.actualClass = new ocl.CylCutter(d, l)
    }
}

export default CylCutter