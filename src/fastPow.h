#ifndef FAST_POW_H
#define FAST_POW_H

#include "floatConst.h"
#include <math.h>               // Some methods calculate exp2 and log2 instead of powf
#include <xmmintrin.h>         // SSE instructions
#include "fastPaulMineiro.h"    // Accurate approximations also work

// Any power (the greater the exponent, the worse the approximation)
float fastPow(const float number, const float exponent) {
    // Need to operate on bit representation
    union {float as_value; uintFlt_t bit_repr;} num;
    num.as_value = number;

    const float COMMON = FLT_BIT_TERM * (LOG_APPROX_CORRECT - FLT_EXP_BIAS);
    num.bit_repr = exponent * num.bit_repr + (exponent - 1) * COMMON;
    //num.bit_repr = exponent * (num.bit_repr + COMMON) - COMMON;   // Alternative form

    // Newton's method to improve guess (uses an exponential!)
    //num.as_value *= 1 - exponent * (1 - number / powf(num.as_value, 1.0f / exponent));

    return num.as_value;
}

__m128 fastPow_vec(const __m128 number, const __m128 exponent) {
    union {__m128 as_value; __m128i bit_repr;} num;
    num.as_value = number;

    const __m128 COMMON = _mm_set1_ps(FLT_BIT_TERM * (LOG_APPROX_CORRECT - FLT_EXP_BIAS));

    __m128 expMulBitRepr =  _mm_mul_ps(exponent, _mm_cvtepi32_ps(num.bit_repr));
    __m128 expTakeOne =     _mm_sub_ps(exponent, _mm_set1_ps(1.0f));
    __m128 expMulCommon =   _mm_mul_ps(expTakeOne, COMMON);
    __m128 result =         _mm_add_ps(expMulBitRepr, expMulCommon);

    num.bit_repr = _mm_cvtps_epi32(result);
    return num.as_value;
}


// Slower but more accurate, especially with larger exponents
float fastPowAlt(const float number, const float exponent) {
    // Need to operate on bit representation
    union {float as_value; uintFlt_t bit_repr;} num;
    num.as_value = number;

    // Extract the components of a float
    const uintFlt_t Mant = num.bit_repr & (FLT_BIT_TERM - 1);
    const uintFlt_t Exp = num.bit_repr & (~0 << FLT_MANT_DIG - 1);  // Include the sign bit

    // Approximation for: BIT_TERM * log2f(Mant / BIT_TERM + 1)
    // log2(x + 1) ≈ 1.356x - 0.3623x², x:(0,1)
    const float LOG2_MANT = Mant * (1.356f - (0.3623f / FLT_BIT_TERM) * Mant);

    num.bit_repr =  exponent * (LOG2_MANT + Exp - FLT_BIT_TERM * FLT_EXP_BIAS)
                    + FLT_BIT_TERM * (FLT_EXP_BIAS - LOG_APPROX_CORRECT);

    return num.as_value;
}

float fastPowAltAlt(const float number, const float exponent) {
    // Need to operate on bit representation
    union {float as_value; uintFlt_t bit_repr;} result;

    // Any fairly accurate (~0.1% error) log2 approx works
    result.bit_repr = (exponent * fastLog2_accurate(number) + (FLT_EXP_BIAS - LOG_APPROX_CORRECT)) * FLT_BIT_TERM;

    return result.as_value;
}



// Does calculations in double precision, so float output is exact
float powViaLogf(const float number, const float exponent) {
    return exp2( (double)exponent * log2(number) );
};

// Paul Mineiro
float fastPowViaLogf(const float number, const float exponent) {
    return fastExp2_accurate( exponent * fastLog2_accurate(number) );
}

// ~23% faster but fails for negatives bases
// #TODO: Why does this happen? (exp * log2(neg) ≈ 730???).
float fastPowViaLogf_A(const float number, const float exponent) {
    return fastExp2_accurate_A( exponent * fastLog2_accurate(number) );
}

#endif