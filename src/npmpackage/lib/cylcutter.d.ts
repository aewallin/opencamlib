import MillingCutter from './millingcutter';
declare class CylCutter extends MillingCutter {
    protected diameter: number;
    protected length: number;
    constructor(diameter: number, length: number);
    serialize(): [string, any[]];
}
export default CylCutter;
