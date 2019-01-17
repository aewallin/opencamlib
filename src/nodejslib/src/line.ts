import ocl from './ocl'

import Point from './point'

class Line {
    actualClass: any

    constructor(p1: Point, p2: Point) {
        this.actualClass = new ocl.Line(p1.actualClass, p2.actualClass)
    }
}

export default Line