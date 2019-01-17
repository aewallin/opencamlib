import ocl from './ocl'

import MillingCutter from './millingcutter'

class BallCutter extends MillingCutter {
    actualClass: any

    constructor(public d: number, public l: number) {
        super()
        this.actualClass = new ocl.BallCutter(d, l)
    }
}

export default BallCutter