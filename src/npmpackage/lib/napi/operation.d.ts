import STLSurf from '../stlsurf';
import MillingCutter from '../millingcutter';
declare class Operation {
    protected actualClass: any;
    setSTL(surface: STLSurf): void;
    setCutter(cutter: MillingCutter): void;
    setSampling(sampling: number): void;
}
export default Operation;
