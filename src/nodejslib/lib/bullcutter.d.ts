import MillingCutter from './millingcutter';
declare class BullCutter extends MillingCutter {
    d: number;
    r: number;
    l: number;
    actualClass: any;
    constructor(d: number, r: number, l: number);
}
export default BullCutter;
