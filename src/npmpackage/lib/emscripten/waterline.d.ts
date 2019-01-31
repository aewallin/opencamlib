import Operation from './operation';
declare class EmscriptenWaterline extends Operation {
    constructor();
    setZ(z: number): void;
    loopsToArray(loops: any): any;
    run(): Promise<{}>;
}
export default EmscriptenWaterline;
