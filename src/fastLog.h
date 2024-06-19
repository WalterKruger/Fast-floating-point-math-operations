#ifndef FAST_LOG_H
#define FAST_LOG_H

#include "floatConst.h"
#include <math.h>

// Natural log (NOTE: More accurate to do fastLog2_accurate(number) / log2f(e))
float fastLog(const float number) {
    // Need to operate on bit representation
    union {float as_value; uintFlt_t bit_repr;} num;
    num.as_value = number;

    const float A = log(2) * FLT_EXP_BIAS - 0.03972;
    const float B = 1.0 / FLT_BIT_TERM * log(2);

    num.as_value = num.bit_repr * B - A;
    //num.as_value = log(2) * (number - 1) + 0.03972;

    // Uses an exponential (!)
    //num.as_value -= (1 - number / exp(num.as_value)) / num.as_value;

    return num.as_value;
}

#endif