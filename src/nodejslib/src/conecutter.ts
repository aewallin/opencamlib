import ocl from './ocl'

import MillingCutter from './millingcutter'

class ConeCutter extends MillingCutter {
    actualClass: any

    constructor(public d: number, public a: number, public l: number) {
        super()
        this.actualClass = new ocl.ConeCutter(d, a, l)
    }
}

export default ConeCutter