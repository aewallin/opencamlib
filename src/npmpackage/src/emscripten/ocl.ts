export interface ModuleOptions {
    /** Absolute URL of the unbundled generated ocl.js, needed by pthread workers. */
    mainScriptUrlOrBlob?: string | Blob
    print?: (message: string) => void
    printErr?: (message: string) => void
}

let modulePromise: Promise<any> | undefined

/** Configure the shared module before constructing operations, or reuse it. */
export default function initialize(options?: ModuleOptions): Promise<any> {
    if (modulePromise) {
        if (options) throw new Error('OpenCAMLib has already been initialized')
        return modulePromise
    }
    modulePromise = require('../../build/ocl')(options || {})
    return modulePromise!
}
