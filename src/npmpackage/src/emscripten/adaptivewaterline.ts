import Operation, { Point3 } from './operation'
class AdaptiveWaterline extends Operation {
    constructor() { super('AdaptiveWaterline') }
    setMinSampling(value: number) {
        return this.whenReady(() => { this.actualClass.setMinSampling(value) })
    }
    getLoops(): Promise<Point3[][]> {
        return this.whenReady(() => this.loopsToArray(this.actualClass.getLoops()))
    }
}
export default AdaptiveWaterline
