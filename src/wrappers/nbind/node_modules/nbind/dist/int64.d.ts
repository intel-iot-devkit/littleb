/** Simple bignum style class for formatting 64-bit integers. */
export declare class Int64 {
    constructor(lo: number, hi: number, sign: boolean);
    fromJS(output: (lo: number, hi: number, sign: boolean) => void): void;
    toString(base: number): string;
    lo: number;
    hi: number;
    sign: boolean;
}
