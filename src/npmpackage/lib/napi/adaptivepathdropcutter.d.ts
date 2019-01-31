import Operation from './operation';
import Path from '../path';
declare class NapiAdaptivePathDropCutter extends Operation {
    constructor();
    setMinSampling(minSampling: number): void;
    setPath(path: Path): void;
    run(): Promise<any>;
}
export default NapiAdaptivePathDropCutter;
