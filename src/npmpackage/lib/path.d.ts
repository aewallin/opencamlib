import Span from './span';
declare class Path {
    protected segments: Span[];
    append(segment: Span): void;
    serialize(): [string, any[]][];
}
export default Path;
