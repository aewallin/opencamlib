import Operation from './operation';
import ocl from './ocl'

class EmscriptenWaterline extends Operation {
    constructor() {
        super()
        this.chain = this.chain.then(() => {
            this.actualClass = new ocl.Waterline()
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
        return new Promise(resolve => {
            this.chain.then(() => {
                this.actualClass.run()
                resolve(this.loopsToArray(this.actualClass.getLoops()))
            })
        })
    }
}

export default EmscriptenWaterline