import Point from './point'
import Span from './span'

class Line extends Span {
    constructor(protected p1: Point, protected p2: Point) {
        super()
    }

    serialize(): [string, any[]] {
        return ['Line', [this.p1, this.p2]]
    }
}

export default Line