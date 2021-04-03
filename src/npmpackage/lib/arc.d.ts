import Span from './span';
import Point from './point';
declare class Arc extends Span {
    protected p1: Point;
    protected p2: Point;
    protected c: Point;
    protected antiClockwise: boolean;
    constructor(p1: Point, p2: Point, c: Point, antiClockwise?: boolean);
    serialize(): [string, any[]];
}
export default Arc;
