import MillingCutter from './millingcutter';
declare class CylCutter extends MillingCutter {
    d: number;
    l: number;
    actualClass: any;
    constructor(d: number, l: number);
}
export default CylCutter;
