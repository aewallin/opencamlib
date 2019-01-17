import Operation from './operation';
declare class Waterline extends Operation {
    protected z?: number;
    setZ(z: number): void;
    getLoops(): any;
    run(): void;
}
export default Waterline;
