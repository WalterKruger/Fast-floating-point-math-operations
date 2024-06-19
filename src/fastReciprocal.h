#ifndef FAST_RECIPROCAL_H
#define FAST_RECIPROCAL_H

#include "floatConst.h"

// On my machine 1.0/n is faster and more accurate, might be useful for embedded though
float fastReciprocal(const float number) {
    // Need to operate on bit representation
    union {float as_value; uintFlt_t bit_repr;} num;
    num.as_value = number;

    //num.bit_repr = POW_APPROX(num.bit_repr, -1);
    const uintFlt_t MAGIC_NUM = -FLT_BIT_TERM * 2 * (LOG_APPROX_CORRECT - FLT_EXP_BIAS);
    num.bit_repr = MAGIC_NUM - num.bit_repr;
    //num.bit_repr = 0x7EF15500 - num.bit_repr;     // Literal form (float32)

    // Newton's method to improve guess
    num.as_value *= 2 - number * num.as_value;
    num.as_value *= 2 - number * num.as_value;    // Two same speed with fused multiply add
    
    return num.as_value;
}

// Hardware instruction (NOTE: 1.0f / number is as fast and more accurate on modern CPUs)
float reciprocalSSE(const float number) {
    __m128 reciprocalVector = _mm_rcp_ss(_mm_load_ss(&number) );
    float approx = _mm_cvtss_f32(reciprocalVector);

    // Improve approx. May not be needed; very accurate already
    //approx *= 2 - number * approx;

    return approx;
}

#endif