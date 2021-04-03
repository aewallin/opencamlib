import Operation from './operation';
declare class AdaptiveWaterline extends Operation {
    constructor();
    setMinSampling(minSampling: number): void;
    setZ(z: number): void;
    loopsToArray(loops: any): any;
    run(): Promise<unknown>;
}
export default AdaptiveWaterline;
