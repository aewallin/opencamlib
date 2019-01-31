import STLSurf from '../stlsurf'
import MillingCutter from '../millingcutter'
import ocl from './ocl'

class Operation {
    protected actualClass: any
    protected chain: Promise<any>

    constructor() {
        this.chain = new Promise((resolve) => {
            ocl.then(() => {
                resolve()
            })
        })
    }

    setSTL(surface: STLSurf) {
        this.chain = this.chain.then(() => {
            if (!surface.triangles) {
                throw new Error('Please provide a valid STLSurf, use STLReader to read a file into it')
            }
            const actualSurface = new ocl.STLSurf()
            surface.triangles.forEach((vertice: [number, number, number][]) => {
                const p1 = new ocl.Point(vertice[0][0], vertice[0][1], vertice[0][2])
                const p2 = new ocl.Point(vertice[1][0], vertice[1][1], vertice[1][2])
                const p3 = new ocl.Point(vertice[2][0], vertice[2][1], vertice[2][2])
                const triangle = new ocl.Triangle(p1, p2, p3)
                actualSurface.addTriangle(triangle)
            })
            this.actualClass.setSTL(actualSurface)
        })
    }

    setCutter(cutter: MillingCutter) {
        this.chain = this.chain.then(() => {
            const cutterDef = cutter.serialize()
            const [cutterType, cutterArgs] = cutterDef
            const Cutter = ocl[cutterType]
            const actualCutter = new Cutter(...cutterArgs)
            this.actualClass.setCutter(actualCutter)
        })
    }

    setSampling(sampling: number) {
        this.chain = this.chain.then(() => {
            this.actualClass.setSampling(sampling)
        })
    }
}

export default Operation