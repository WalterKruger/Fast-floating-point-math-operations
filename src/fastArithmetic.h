#ifndef FAST_ARITHMETIC_H
#define FAST_ARITHMETIC_H

#include "floatConst.h"

// As fast as a multiply on modern CPUs
float fastMultiply(const float factor_a, const float factor_b) {
    // Need to operate on bit representations
    union {float as_value; uintFlt_t bit_repr;}
    a = {.as_value=factor_a}, b = {.as_value=factor_b}, result;

    // Calculated during compile time
    const uintFlt_t MAGIC_NUM = -FLT_BIT_TERM * ((double)LOG_APPROX_CORRECT - FLT_EXP_BIAS);
    result.bit_repr = a.bit_repr + b.bit_repr - MAGIC_NUM;

    return result.as_value;
}

float fastDivision(const float numerator, const float denominator) {
    // Need to operate on bit representations
    union {float as_value; uintFlt_t bit_repr;}
    nume = {.as_value=numerator}, denom = {.as_value=denominator}, result;

    // Calculated during compile time
    const uintFlt_t MAGIC_NUM = -FLT_BIT_TERM * ((double)LOG_APPROX_CORRECT - FLT_EXP_BIAS);
    result.bit_repr = nume.bit_repr - denom.bit_repr + MAGIC_NUM;

    return result.as_value;
}

float multiByPowOf2(const float number, const uint8_t exponent) {
    // Need to operate on bit representation
    union {float as_value; uintFlt_t bit_repr;} num;
    num.as_value = number;

    num.bit_repr += exponent << (FLT_MANT_DIG - 1);
    
    return num.as_value;
}

#endif