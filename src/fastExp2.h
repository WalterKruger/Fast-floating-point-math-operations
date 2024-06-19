#ifndef FAST_EXP2_H
#define FAST_EXP2_H

#include "floatConst.h"

float fastExp2(const float exponent) {
    // Need to operate on bit representation
    union {float as_value; uintFlt_t bit_repr;} result;

    /* Float32 bit_repr =   M + (2^23) * E
     *
     * log2(float32 val.) = log2( 1 + M / (2^23) ) + E-127
     * ≈ 1/(2^23) * (M + (2^23) * E) + μ - 127   {log2(x+1) ≈ x + μ, x:(0,1)}
     * 
     * log2( 2^pow ) ≈ 1/(2^23) * solution_bit_repr + μ - 127
     * solution_bit_repr ≈ (pow - μ + 127) * 2^23
    */
    result.bit_repr = (exponent + (FLT_EXP_BIAS - LOG_APPROX_CORRECT)) * FLT_BIT_TERM;
    
    return result.as_value;
}

// Negative inputs don't work...
float fastExp2_alt(const float input) {
    const uintFlt_t int_part = input;

    union { uintFlt_t bit_repr; float as_value; } int_result;
    int_result.bit_repr = (int_part + FLT_EXP_BIAS) << (FLT_MANT_DIG - 1);

    const float frac_part = input - int_part;
    const float frac_result =  frac_part * ( 0.336f * frac_part + 0.659f) + 1.0015f;

    return int_result.as_value * frac_result;
}

// Very accurate and slightly faster than exp2f()
inline float exp2_asm(const float input) {
    uint64_t int_part = input;
    const float frac_part = input - int_part;

    # ifdef __SIZEOF_INT128__
        float int_result = (__uint128_t)1 << int_part;
    # else
        #error Unsupported 128-bit integer type
    # endif

    float frac_result;
    __asm__ (
        "F2XM1"     // Takes arguments from FPU stack
        : "=t" (frac_result)
        : "0" (frac_part)
    );
    //printf("\tInt part: %llu, Int result: %.4lf\n", int_part, int_result);
    return int_result * (frac_result + 1);
}

// Faster than above
float exp2_asm_alt(const float input) {
    // Has to be int8 for performance reasons
    const int8_t int_part = input;

    union { uintFlt_t bit_repr; float as_value; } inp;
    inp.bit_repr = (int_part + FLT_EXP_BIAS) << (FLT_MANT_DIG - 1);

    float frac_result;
    __asm__ (
        "F2XM1"     // Takes arguments from FPU stack
        : "=t" (frac_result)
        : "0" (input - int_part)
    );
    return inp.as_value * (frac_result + 1);
}

float fastExp2_bits(const float input) {
    // Expontent only has 8-bits, but conversion is faster
    const intFlt_t int_part = input;

    union { uintFlt_t bit_repr; float as_value; } inp;
    inp.bit_repr = (int_part + FLT_EXP_BIAS) << (FLT_MANT_DIG - 1);

    // Taylor series w/t 4 terms: 2^x - 1 = ∑ [(x*ln⁡2)^n / n!]
    // Horner's method
    
    const float frac_x_ln2 = (input - int_part) * logf(2);
    const float frac_result = frac_x_ln2 * (
                                1.0f + frac_x_ln2 * (
                                    0.5f + frac_x_ln2 * (
                                        1.0f/6 + frac_x_ln2 * (
                                            1.0f / 24 + frac_x_ln2 *
                                            (1.0f/120) ))));
    /*
    // Positive inputs (~0.0035%), Negative inputs (~0.039%);
    const float frac_inp = input - int_part;
    const float frac_result = (input > 0)? 
                                27.704226690769845416f / (4.8416702244134115171f - frac_inp)
                                - 0.48942480030516666506f * frac_inp
                                - 5.7220391320516093836f
                                :
                                27.525417f / (4.953134f - frac_inp)
                                - 0.434024f * frac_inp
                                - 5.557512f;

*/
    
    return inp.as_value * (frac_result + 1);
}

#endif