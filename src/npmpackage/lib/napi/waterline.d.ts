import Operation from './operation';
declare class NodeJSWaterline extends Operation {
    constructor();
    setZ(z: number): void;
    run(): Promise<any>;
}
export default NodeJSWaterline;
