import { buildPath } from '../native'
import Operation from './operation'
import Path from '../path'
import ocl from './ocl'

class NapiPathDropCutter extends Operation {
    constructor() {
        super()
        this.actualClass = new ocl.PathDropCutter()
    }

    setPath(path: Path) {
        this.actualClass.setPath(buildPath(ocl, path))
    }

    setZ(z: number) {
        this.actualClass.setZ(z)
    }

    run() {
        this.actualClass.run()
    }

    getPoints(): [number, number, number][] { return this.getCLPoints() }

    getCLPoints(): [number, number, number][] {
        return this.actualClass.getCLPoints()
    }
}

export default NapiPathDropCutter