import MillingCutter from './millingcutter'

class BallCutter extends MillingCutter {
    constructor(protected diameter: number, protected length: number) {
        super()
    }

    serialize(): [string, any[]] {
        return ['BallCutter', [this.diameter, this.length]]
    }
}

export default BallCutter