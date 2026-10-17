import Triangle from "./triangle"

type Point = [number, number, number]
type Triangles = Point[][]

class STLSurf {
    public triangles: Triangles = []

    addTriangle(triangle: Triangle) {
        this.triangles.push(triangle.serialize()[1].map(point => [point.x, point.y, point.z] as Point))
    }

    serialize(): Triangles | undefined {
        return this.triangles
    }
}

export default STLSurf