/// <reference types="node" />
import STLSurf from '../stlsurf';
declare class STLReader {
    constructor(contents: string | Buffer, surface: STLSurf);
}
export default STLReader;
