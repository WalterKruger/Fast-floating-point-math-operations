#ifndef FAST_SQRT_H
#define FAST_SQRT_H

#include "floatConst.h"
# include <xmmintrin.h>     // SSE instructions

// The original Quake "q_rsqrt" function
float fastInvSqrRoot(const float number)
{    
    // Need to operate on a float's bits, so prune type
    union {float as_value; uintFlt_t bit_repr;} num;
    num.as_value = number;

    /* Float32 value =      ( 1 + Mantissa / (2^23) ) * 2^(Exponent - 127)
     * Float32 bit_repr =   M + (2^23) * E
     *
     * log2(float32 val.) = log2( 1 + M / (2^23) ) + E-127
     * ≈ 1/(2^23) * (M + (2^23) * E) - 127              {includes bit representation!}
     * 
     * log2( 1/√val ) ≈ -1/2 * ( 1/(2^23) * bit_repr - 127) )        {log(x^n) = n*log(x)}
     * 1/√val ≈ 1.5 * 2^23 * 127 - 1/2 * bit_repr 
    */
    const intFlt_t MAGIC_NUM = 1.5 * FLT_BIT_TERM * (FLT_EXP_BIAS - LOG_APPROX_CORRECT);
    num.bit_repr = MAGIC_NUM - (num.bit_repr >> 1);      // - 1/2 * n = - (n >> 1)

    // Newton's method to improve guess (can be ommited)
    num.as_value *= 1.5f - number * 0.5f * num.as_value * num.as_value;

    return num.as_value;
}

// Same as above but using a double
double fastInvSqrRoot_DBL(const double number) {    
    // Need to operate on a float's bits, so prune type
    union {double as_value; uintDbl_t bit_repr;} num;
    num.as_value = number;

    const uintDbl_t MAGIC_NUM = 1.5 * DBL_BIT_TERM * (DBL_EXP_BIAS - LOG_APPROX_CORRECT);
    num.bit_repr = MAGIC_NUM - (num.bit_repr >> 1);      // - 1/2 * n ≈ - (n >> 1)

    // Newton's method to improve guess (can be ommited)
    num.as_value *= 1.5f - number * 0.5f * num.as_value * num.as_value;

    return num.as_value;
}



// Hardware instruction (very accurate)
float invSqrRootSSE(const float number) {
    __m128 rSqrRootVector = _mm_rsqrt_ss(_mm_load_ss(&number));
    return _mm_cvtss_f32(rSqrRootVector);
}

// Marginally faster than sqrt()
float fastSqrRoot(const float numberToRoot) {
    // Need to operate on a float's bits, so prune type
    union {float as_value; uintFlt_t bit_repr;} num;
    num.as_value = numberToRoot;

    // log2(float32) ≈ 1/(2^23) * bit_repr - 127
    // √float32 ≈ ((1/(2^23) * bit_repr - 127) / 2 + 127) * 2^23
    //num.bit_repr = (num.bit_repr - FLT_BIT_TERM >> 1) + (FLT_EXP_BIAS + 1 << FLT_MANT_DIG - 2);
    //num.bit_repr -= 0x4B0D2;    // Reduces maximum relative error

    // More accurate than above
    const uintFlt_t MAGIC_NUM = FLT_BIT_TERM * (1.0 / 2 - 1) * ((double)LOG_APPROX_CORRECT - FLT_EXP_BIAS);
    num.bit_repr = (num.bit_repr >> 1) + MAGIC_NUM;
    //num.bit_repr = (num.bit_repr >> 1) + 0x1FBD3F7C;    // Literal form (float32)

    // Heron's method to improve guess (can be ommited)
    //num.as_value = 0.5f * (num.as_value + numberToRoot / num.as_value);

    return num.as_value;
}

__m128 fastSqrt_vec(__m128 float_vec) {
    union {__m128 as_value; __m128i bit_repr;} num;
    num.as_value = float_vec;

    num.bit_repr = _mm_srli_epi32(num.bit_repr, 1);
    num.bit_repr = _mm_add_epi32(num.bit_repr, _mm_set1_epi32(0x1FBD3F7C));

    return num.as_value;
}

// Marginally faster
float fastSqrt_xmm(float number) {
    union {__m128 as_value; __m128i bit_repr;} num;
    num.as_value = _mm_set_ss(number);

    num.bit_repr = _mm_srli_epi32(num.bit_repr, 1);
    num.bit_repr = _mm_add_epi32(num.bit_repr, _mm_set1_epi32(0x1FBD3F7C));

    return _mm_cvtss_f32(num.as_value);
}

// Hardware instruction (not an approximation!)
float sqrRootSSE(const float number) {
    __m128 sqrRootVector = _mm_sqrt_ss(_mm_load_ss(&number));
    return _mm_cvtss_f32(sqrRootVector);
}

#endif