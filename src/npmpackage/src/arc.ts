import Span from './span'
import Point from './point'

class Arc extends Span {
    // create an arc from point p1 to point p2 with center c and direction dir.
    constructor(protected p1: Point, protected p2: Point, protected c: Point, protected antiClockwise: boolean = false) {
        super()
    }

    serialize(): [string, any[]] {
        return ['Arc', [this.p1, this.p2, this.c, this.antiClockwise]]
    }
}

export default Arc