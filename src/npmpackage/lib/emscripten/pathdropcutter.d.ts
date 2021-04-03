import Path from '../path';
import Operation from './operation';
declare class PathDropCutter extends Operation {
    constructor();
    setPath(path: Path): void;
    run(): Promise<any>;
}
export default PathDropCutter;
