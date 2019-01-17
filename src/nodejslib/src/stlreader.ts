import ocl from './ocl'

import STLSurf from './stlsurf'

class STLReader {
    actualClass: any

    constructor(filepath: string, surface: STLSurf) {
        this.actualClass = new ocl.STLReader(filepath, surface.actualClass)
    }
}

export default STLReader