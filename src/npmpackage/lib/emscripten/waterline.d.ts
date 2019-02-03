import Operation from './operation';
declare class EmscriptenWaterline extends Operation {
    constructor();
    setZ(z: number): void;
    run(): Promise<{}>;
}
export default EmscriptenWaterline;
