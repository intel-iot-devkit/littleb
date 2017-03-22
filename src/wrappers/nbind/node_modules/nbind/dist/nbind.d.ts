import { SignatureType } from './common';
/** Compiled C++ binary type and path. */
export interface FindModuleSpec {
    type: 'node' | 'emcc';
    ext: string;
    name: string;
    path?: string;
}
export interface ModuleSpec extends FindModuleSpec {
    path: string;
}
/** Any class constructor. */
export declare type ClassType = {
    new (...args: any[]): any;
};
export interface DefaultExportType {
    [key: string]: any;
    locateFile?(name: string): string;
    onRuntimeInitialized?(): void;
    ccall?(name: string, returnType?: string, argTypes?: string[], args?: any[]): any;
    _nbind_value?(name: string, proto: ClassType): void;
    NBind?: {
        bind_value(name: string, proto: ClassType): void;
    };
}
export declare class Binding<ExportType extends DefaultExportType> {
    [key: string]: any;
    queryType?<Result>(id: number, outTypeDetail: (kind: number, ...args: any[]) => Result): Result;
    /** Bind a value type (class with a fromJS method) to an equivalent C++ type. */
    bind: (name: string, proto: ClassType) => void;
    reflect: (outPrimitive: (id: number, size: number, flag: number) => void, outType: (id: number, name: string) => void, outClass: (id: number, name: string) => void, outSuper: (classId: number, superIdList: number[]) => void, outMethod: (classId: number, name: string, kind: SignatureType, argTypeList: number[], policyList: string[]) => void) => void;
    toggleLightGC: (enable: boolean) => void;
    binary: ModuleSpec;
    /** Exported API of a C++ library compiled for nbind. */
    lib: ExportType;
}
export declare type FindCallback = (err: any, result?: ModuleSpec) => any;
/** Find compiled C++ binary under current working directory. */
export declare function find(cb?: FindCallback): ModuleSpec;
/** Find compiled C++ binary under given path. */
export declare function find(basePath: string, cb?: FindCallback): ModuleSpec;
export declare type InitCallback<ExportType extends DefaultExportType> = (err: any, result?: Binding<ExportType>) => any;
/** Initialize compiled C++ binary under current working directory. */
export declare function init<ExportType extends DefaultExportType>(cb?: InitCallback<ExportType>): Binding<ExportType>;
/** Initialize compiled C++ binary under given path. */
export declare function init<ExportType extends DefaultExportType>(basePath: string, cb?: InitCallback<ExportType>): Binding<ExportType>;
/** Initialize compiled C++ binary under given path and merge its API to given
  * object, which may contain options for Emscripten modules. */
export declare function init<ExportType extends DefaultExportType>(basePath: string, lib: ExportType, cb?: InitCallback<ExportType>): Binding<ExportType>;
