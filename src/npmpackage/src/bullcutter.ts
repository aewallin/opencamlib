import MillingCutter from './millingcutter'

class BullCutter extends MillingCutter {
    constructor(protected diameter: number, protected radius: number, protected length: number) {
        super()
    }

    serialize(): [string, any[]] {
        return ['BullCutter', [this.diameter, this.radius, this.length]]
    }
}

export default BullCutter