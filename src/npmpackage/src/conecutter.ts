import MillingCutter from './millingcutter'

class ConeCutter extends MillingCutter {
    // create a ConeCutter with specified maximum diameter and cone-angle
    // for a 90-degree cone specify the half-angle  angle= pi/4
    constructor(protected diameter: number, protected angle: number, protected length = 10) {
        super()
    }

    serialize(): [string, any[]] {
        return ['ConeCutter', [this.diameter, this.angle, this.length]]
    }
}

export default ConeCutter