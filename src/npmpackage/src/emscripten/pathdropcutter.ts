import Path from '../path'
import Operation, { Point3 } from './operation'

class PathDropCutter extends Operation {
    constructor() { super('PathDropCutter') }
    setPath(path: Path) { return this.setNativePath(path) }
    getCLPoints(): Promise<Point3[]> {
        return this.whenReady(() => this.pointsToArray(this.actualClass.getCLPoints()))
    }
    getPoints(): Promise<Point3[]> { return this.getCLPoints() }
}
export default PathDropCutter
