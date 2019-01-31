type Point = [number, number, number]
type Triangles = Point[][]

class STLSurf {
    public triangles?: Triangles

    serialize(): Triangles | undefined {
        return this.triangles
    }
}

export default STLSurf