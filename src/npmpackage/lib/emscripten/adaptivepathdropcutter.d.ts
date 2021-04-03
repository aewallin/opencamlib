import Path from '../path';
import Operation from './operation';
declare class EmscriptenAdaptivePathDropCutter extends Operation {
    constructor();
    setMinSampling(minSampling: number): void;
    setPath(path: Path): void;
    run(): Promise<any>;
}
export default EmscriptenAdaptivePathDropCutter;
