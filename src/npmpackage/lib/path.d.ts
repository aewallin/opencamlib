import Span from './span';
declare class Path {
    protected segments: Span[];
    append(line: Span): void;
    serialize(): [string, any[]][];
}
export default Path;
