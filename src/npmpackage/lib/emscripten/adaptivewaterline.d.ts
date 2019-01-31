import Operation from './operation';
declare class EmscriptenAdaptiveWaterline extends Operation {
    constructor();
    setMinSampling(minSampling: number): void;
    setZ(z: number): void;
    loopsToArray(loops: any): any;
    run(): Promise<{}>;
}
export default EmscriptenAdaptiveWaterline;
