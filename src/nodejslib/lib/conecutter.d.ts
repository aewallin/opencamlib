import MillingCutter from './millingcutter';
declare class ConeCutter extends MillingCutter {
    d: number;
    a: number;
    l: number;
    actualClass: any;
    constructor(d: number, a: number, l: number);
}
export default ConeCutter;
