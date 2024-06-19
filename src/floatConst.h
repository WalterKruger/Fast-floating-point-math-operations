#ifndef FLOAT_CONST_H
#define FLOAT_CONST_H


#include <float.h>
#include "intsizedfloat.h"

# define PI 3.1415926535
# define EULER 2.7182818284     // More syntactically meaningful than exp(1)

#define FLT_BIT_TERM ( (intFlt_t)1 << FLT_MANT_DIG - 1 )  // Float bit repr. := bitTerm * Exponent + Mantissa
#define FLT_EXP_BIAS ( FLT_MAX_EXP - 1)

#define DBL_BIT_TERM ( (intDbl_t)1 << DBL_MANT_DIG - 1 )
#define DBL_EXP_BIAS ( DBL_MAX_EXP - 1)

#define LDBL_BIT_TERM ( (intLDbl_t)1 << LDBL_MANT_DIG - 1 )
#define LDBL_EXP_BIAS ( LDBL_MAX_EXP - 1)



// Error correction for approximation: log2(x+1) ≈ x, x:(0,1)
//const float LOG_APPROX_CORRECT = 0.05730;   // ∫[0,1] ( log2(1+x) - x )dx
static const float LOG_APPROX_CORRECT = 0.043;


#endif