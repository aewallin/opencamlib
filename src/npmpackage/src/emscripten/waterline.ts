import Operation, { Point3 } from './operation'
class Waterline extends Operation {
    constructor() { super('Waterline') }
    getLoops(): Promise<Point3[][]> {
        return this.whenReady(() => this.loopsToArray(this.actualClass.getLoops()))
    }
}
export default Waterline
