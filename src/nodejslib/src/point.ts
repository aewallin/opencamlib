import ocl from './ocl'

class Point {
    actualClass: any

    constructor(x: number, y: number, z: number) {
        this.actualClass = new ocl.Point(x, y, z)
    }
}

export default Point