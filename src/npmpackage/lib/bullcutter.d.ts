import MillingCutter from './millingcutter';
declare class BullCutter extends MillingCutter {
    protected diameter: number;
    protected radius: number;
    protected length: number;
    constructor(diameter: number, radius: number, length: number);
    serialize(): [string, any[]];
}
export default BullCutter;
