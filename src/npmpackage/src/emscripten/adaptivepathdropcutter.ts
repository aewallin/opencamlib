import Path from '../path'
import Operation, { Point3 } from './operation'

class AdaptivePathDropCutter extends Operation {
    constructor() { super('AdaptivePathDropCutter') }
    setPath(path: Path) { return this.setNativePath(path) }
    setMinSampling(value: number) {
        return this.whenReady(() => { this.actualClass.setMinSampling(value) })
    }
    getCLPoints(): Promise<Point3[]> {
        return this.whenReady(() => this.pointsToArray(this.actualClass.getCLPoints()))
    }
    getPoints(): Promise<Point3[]> { return this.getCLPoints() }
}
export default AdaptivePathDropCutter
