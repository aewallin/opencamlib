import Operation from './operation';
import Path from './path';
declare class AdaptivePathDropCutter extends Operation {
    protected minSampling?: number;
    protected path?: Path;
    setMinSampling(minSampling: number): void;
    getCLPoints(): any;
    setPath(path: Path): void;
    run(): void;
}
export default AdaptivePathDropCutter;
