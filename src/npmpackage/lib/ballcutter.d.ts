import MillingCutter from './millingcutter';
declare class BallCutter extends MillingCutter {
    protected diameter: number;
    protected length: number;
    constructor(diameter: number, length: number);
    serialize(): [string, any[]];
}
export default BallCutter;
