declare type Point = [number, number, number];
declare type Triangles = Point[][];
declare class STLSurf {
    triangles?: Triangles;
    serialize(): Triangles | undefined;
}
export default STLSurf;
