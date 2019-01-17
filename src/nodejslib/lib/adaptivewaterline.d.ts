import Operation from './operation';
declare class AdaptiveWaterline extends Operation {
    protected z?: number;
    protected minSampling?: number;
    setZ(z: number): void;
    setMinSampling(minSampling: number): void;
    getLoops(): any;
    run(): void;
}
export default AdaptiveWaterline;
