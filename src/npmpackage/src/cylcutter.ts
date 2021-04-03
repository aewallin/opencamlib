import MillingCutter from './millingcutter'

class CylCutter extends MillingCutter {
    constructor(protected diameter: number, protected length: number) {
        super()
    }

    serialize(): [string, any[]] {
        return ['CylCutter', [this.diameter, this.length]]
    }
}

export default CylCutter