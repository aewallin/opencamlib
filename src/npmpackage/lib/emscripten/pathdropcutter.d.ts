import Path from '../path';
import Operation from './operation';
declare class EmscriptenPathDropCutter extends Operation {
    constructor();
    setPath(path: Path): void;
    run(): Promise<any>;
}
export default EmscriptenPathDropCutter;
