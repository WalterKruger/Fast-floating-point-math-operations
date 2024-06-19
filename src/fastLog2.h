#ifndef FAST_LOG2_H
#define FAST_LOG2_H

#include "floatConst.h"

static inline float fastLog2(const float number) {
    // Need to operate on bit representation
    union {float as_value; uintFlt_t bit_repr;} num;
    num.as_value = number;

    const float DIV_BIT_TERM = 1.0 / FLT_BIT_TERM;  // Calculated during compile time
    num.as_value = DIV_BIT_TERM * num.bit_repr + (LOG_APPROX_CORRECT - FLT_EXP_BIAS);

    return num.as_value;
}

// Much more accurate than above (Still ill conditioned near 1)
float fastLog2Bits(const float number) {
    union {float as_value; uintFlt_t bit_repr;} num;
    num.as_value = number;

    // Float's exponent equals ⌊log2(x)⌋
    const intFlt_t int_part = (num.bit_repr >> FLT_MANT_DIG - 1) - FLT_EXP_BIAS;

    // Set exponent so mantissa represents range (1,2) (∴log2 range: (0,1))
    num.bit_repr &= FLT_BIT_TERM - 1;
    num.bit_repr = num.bit_repr | (FLT_EXP_BIAS << FLT_MANT_DIG - 1);

    // More accurate approximation for log2(x + 1), for {0 < x < 1}
    num.as_value -= 1;
    const float  fractional_part = num.as_value * (1.356f - 0.3623f * num.as_value);

    return int_part + fractional_part;
}

// Similar to fastLog2, but more accurate
float fastLog2Alt(const float number) {
    union {float as_value; uintFlt_t bit_repr;} num;
    num.as_value = number;

    // Extract the components of a float
    const uintFlt_t Mant = num.bit_repr & (FLT_BIT_TERM - 1);
    const uintFlt_t Exp = num.bit_repr >> (FLT_MANT_DIG - 1);

    const float MAN_DIV_TERM = (float)Mant / FLT_BIT_TERM;
    return MAN_DIV_TERM * (1.356f - 0.3623f * MAN_DIV_TERM) + Exp - FLT_EXP_BIAS;

    // More accurate, but slower
    //return MAN_DIV_TERM * (1.4088f - MAN_DIV_TERM * (0.49328f - 0.086f * MAN_DIV_TERM * MAN_DIV_TERM)) + Exp - FLT_EXP_BIAS;
    //return 3.3739f * Mant / (Mant + FLT_BIT_TERM * 2.3791f) + Exp - FLT_EXP_BIAS;
}


float log2_fpu(const float number) {
    float result;

    __asm__ (
        "fyl2x"     // Takes argumenmts from FPU stack
        : "=t" (result)
        : "0" (number), [constant] "u" (1.0f)
    );
    return result;
}



#endif