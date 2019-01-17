import STLSurf from './stlsurf';
import CylCutter from './cylcutter';
import BallCutter from './ballcutter';
import BullCutter from './bullcutter';
import ConeCutter from './conecutter';
declare type Cutter = CylCutter | BallCutter | BullCutter | ConeCutter;
declare class Operation {
    protected surface?: STLSurf;
    protected cutter?: Cutter;
    protected sampling?: number;
    actualClass: any;
    setSTL(surface: STLSurf): void;
    setCutter(cutter: Cutter): void;
    setSampling(sampling: number): void;
    protected setCutterOnActualClass(): void;
}
export default Operation;
