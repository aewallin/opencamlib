import STLSurf from '../stlsurf'
import stl from 'stl'

class STLReader {
    constructor(contents: string, surface: STLSurf) {
        const obj = stl.toObject(contents)
        surface.triangles = obj.facets.map((facet: any) => facet.verts)
    }
}

export default STLReader