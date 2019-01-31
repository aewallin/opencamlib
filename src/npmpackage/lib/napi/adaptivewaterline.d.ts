import Operation from './operation';
declare class NodeJSAdaptiveWaterline extends Operation {
    constructor();
    setZ(z: number): void;
    setMinSampling(minSampling: number): void;
    run(): Promise<any>;
}
export default NodeJSAdaptiveWaterline;
