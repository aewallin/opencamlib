import Operation from './operation';
import opencamlib from './ocl'

class AdaptiveWaterline extends Operation {
    constructor() {
        super()
        this.chain = this.chain.then(() => {
            return opencamlib.then((ocl: any) => {
                this.actualClass = new ocl.AdaptiveWaterline()
            })
        })
    }

    setMinSampling(minSampling: number) {
        this.chain = this.chain.then(() => {
            this.actualClass.setMinSampling(minSampling)
        })
    }

    setZ(z: number) {
        this.chain = this.chain.then(() => {
            this.actualClass.setZ(z)
        })
    }

    loopsToArray(loops: any): any {
        const result = []
        for (var i = 0; i < loops.size(); i++) {
            const loop = loops.get(i)
            const points = []
            for (var j = 0; j < loop.size(); j++) {
                const point = loop.get(j)
                points.push([point.x, point.y, point.z] as [number, number, number])
            }
            result.push(points)
        }
        return result
    }

    run() {
        this.chain = this.chain.then(() => {
            this.actualClass.run()
        })
    }

    getLoops() {
        this.chain = this.chain.then(() => {
            return this.loopsToArray(this.actualClass.getLoops())
        })
        return this.chain
    }
}

export default AdaptiveWaterline