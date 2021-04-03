import Operation from './operation';
import Path from '../path';
declare class NapiPathDropCutter extends Operation {
    constructor();
    setPath(path: Path): void;
    run(): Promise<any>;
}
export default NapiPathDropCutter;
