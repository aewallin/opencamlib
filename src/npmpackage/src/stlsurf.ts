import Triangle from "./triangle"

type Point = [number, number, number]
type Triangles = Point[][]

class STLSurf {
    public triangles?: Triangles

    addTriangle(triangle: Triangle) {
        this.triangles?.push(triangle.serialize()[1])
    }

    serialize(): Triangles | undefined {
        return this.triangles
    }
}

export default STLSurf