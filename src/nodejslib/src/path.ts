import ocl from './ocl'

import Line from './line'

class Path {
    actualClass: any

    constructor() {
        this.actualClass = new ocl.Path()
    }

    append(line: Line) {
        this.actualClass.append(line.actualClass)
    }
}

export default Path