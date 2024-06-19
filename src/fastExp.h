#ifndef FAST_EXP_H
#define FAST_EXP_H

#include "floatConst.h"

float fastExp(const float exponent) {
    // Need to operate on bit representation
    union {float as_value; uintFlt_t bit_repr;} result;

    const float LOG2E_TERM = log2(EULER) * FLT_BIT_TERM;
    result.bit_repr = exponent * LOG2E_TERM + FLT_BIT_TERM * (FLT_EXP_BIAS - LOG_APPROX_CORRECT);
    
    return result.as_value;
}

#endif