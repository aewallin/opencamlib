import MillingCutter from './millingcutter';
declare class ConeCutter extends MillingCutter {
    protected diameter: number;
    protected angle: number;
    protected length: number;
    constructor(diameter: number, angle: number, length?: number);
    serialize(): [string, any[]];
}
export default ConeCutter;
