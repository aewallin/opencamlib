import Point from './point';
import Span from './span';
declare class Line extends Span {
    protected p1: Point;
    protected p2: Point;
    constructor(p1: Point, p2: Point);
    serialize(): [string, any[]];
}
export default Line;
