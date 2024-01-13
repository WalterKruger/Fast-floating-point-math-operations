# include <stdio.h>
# include <float.h>         // Create magic numbers
#include <xmmintrin.h>     // SSE instructions
# include "intsizedfloat.h" // To bit-shift a float
# include <time.h>          // Performance messuring

# include <math.h>          // For comparasion to approximation
# define PI 3.1415926535
# define EULER 2.7182818284     // More syntactically meaningful than exp(1)

#define FLT_BIT_TERM ( (intFlt_t)1 << FLT_MANT_DIG - 1 )  // Float bit repr. := bitTerm * Exponent + Mantissa
#define FLT_EXP_BIAS ( FLT_MAX_EXP - 1)

#define DBL_BIT_TERM ( (intDbl_t)1 << DBL_MANT_DIG - 1 )
#define DBL_EXP_BIAS ( DBL_MAX_EXP - 1)

// Error correction for approximation: log2(x+1) ≈ x, x:(0,1)
const float LOG_APPROX_CORRECT = 0.05730;   // ∫[0,1] ( log2(1+x) - x )dx

// General power approximation
#define POW_APPROX(bit_repr, exponent) ( exponent * bit_repr + FLT_BIT_TERM * (exponent - 1)*(LOG_APPROX_CORRECT - FLT_EXP_BIAS) )


// This approximation serves as the basis for all others
// NOTE: Ill conditioned near 1
float fastLog2(const float number) {
    // Need to operate on bit representation
    union {float as_value; uintFlt_t bit_repr;} num;
    num.as_value = number;

    const float DIV_BIT_TERM = 1.0 / FLT_BIT_TERM;  // Calculated during compile time
    num.as_value = DIV_BIT_TERM * num.bit_repr + LOG_APPROX_CORRECT - FLT_EXP_BIAS;

    return num.as_value;
}

// Much more accurate than above (Still ill conditioned near 1)
float fastLog2Bits(const float number) {
    union {float as_value; uintFlt_t bit_repr;} num;
    num.as_value = number;

    // Float's exponent equals ⌊log2(x)⌋
    const intFlt_t int_part = (num.bit_repr >> FLT_MANT_DIG - 1) - FLT_EXP_BIAS;

    // Clear out non-mantissa bits
    num.bit_repr &= FLT_BIT_TERM - 1;
    // Set exponent so mantissa represents range (1,2) (∴log2 range: (0,1))
    num.bit_repr = num.bit_repr | (FLT_EXP_BIAS << FLT_MANT_DIG - 1);

    // More accurate approximation for log2(x + 1), for {0 < x < 1}
    num.as_value -= 1;
    const float  fractional_part = num.as_value * (1.356f - 0.3623f * num.as_value);

    return int_part + fractional_part;
    //return int_part + log2f(Mant * 1.0f / FLT_BIT_TERM + 1);
}

// Similar to the above
float fastLog2Alt(const float number) {
    union {float as_value; uintFlt_t bit_repr;} num;
    num.as_value = number;

    // Extract the components of a float
    const uintFlt_t Mant = num.bit_repr & (FLT_BIT_TERM - 1);
    const uintFlt_t Exp = num.bit_repr >> (FLT_MANT_DIG - 1);

    const float MAN_DIV_TERM = (float)Mant / FLT_BIT_TERM;
    //return MAN_DIV_TERM * (1.356f - 0.3623f * MAN_DIV_TERM) + Exp - FLT_EXP_BIAS;

    // More accurate, but slower
    return MAN_DIV_TERM * (1.4088f - MAN_DIV_TERM * (0.49328f - 0.086f * MAN_DIV_TERM * MAN_DIV_TERM)) + Exp - FLT_EXP_BIAS;
    //return 3.3739f * Mant / (Mant + FLT_BIT_TERM * 2.3791f) + Exp - FLT_EXP_BIAS;
}


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
    //num.as_value *= 1.5f - number * 0.5f * num.as_value * num.as_value;

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
    //num.bit_repr = (num.bit_repr >> 1) + 0x1FBC5532;    // Literal form (float32)

    // Heron's method to improve guess (can be ommited)
    num.as_value = 0.5f * (num.as_value + numberToRoot / num.as_value);

    return num.as_value;
}

// Hardware instruction (not an approximation!)
float sqrRootSSE(const float number) {
    __m128 sqrRootVector = _mm_sqrt_ss(_mm_load_ss(&number));
    return _mm_cvtss_f32(sqrRootVector);
}

// Any power (the greater the exponent, the worse the approximation)
float fastPow(const float number, const float exponent) {
    // Need to operate on bit representation
    union {float as_value; uintFlt_t bit_repr;} num;
    num.as_value = number;

    num.bit_repr = POW_APPROX(num.bit_repr, exponent);

    // Newton's method to improve guess (uses an exponential!)
    //num.as_value *= 1 - exponent * (1 - number / fastPowAlt(num.as_value, 1.0f / exponent));

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

float fastInvRootApprox(const float number, const uint8_t nthRoot) {
    const float nthRootRecip = 1.0f / nthRoot;

    // Need to operate on bit representation
    union {float as_value; uintFlt_t bit_repr;} num;
    num.as_value = number;

    //num.bit_repr = POW_APPROX(num.bit_repr, -nthRootRecip);
    // Slightly more accurate than the general form
    num.bit_repr = nthRootRecip * ((nthRoot + 1) * FLT_BIT_TERM * (FLT_EXP_BIAS - LOG_APPROX_CORRECT) - num.bit_repr);

    // Need to calculate difference between inverse guess and original input
    float inverseApproximation = 1;
    for (uint8_t i=0; i < nthRoot; i++) inverseApproximation *= num.as_value;
    
    // Newton's method to improve guess
    num.as_value *= 1 + nthRootRecip * (1 - number * inverseApproximation); 
    
    return num.as_value;
}

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

// Natural log (NOTE: More accurate to do fastLog2Bits(number) / log2f(e))
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

// Inaccurate for smaller inputs (< 2.5)
float fastLogGamma(float number) {
    /*
    number -= 1;
    
    union {float as_value; uintFlt_t bit_repr;} num;
    num.as_value = number;
    
    // Based on Lanczos approximation, worse accuracy
    const float A = 11.35;
    const float C = log(2) / FLT_BIT_TERM;
    const float D = log(2) * FLT_EXP_BIAS - 0.03972 + 1;
    const float B = log(2 * PI) - D + 1;

    const float C_bitRepr = C * num.bit_repr;
    return 0.5f * (B + C_bitRepr) + number * (C_bitRepr - D) + 1.0f / (A * number + 1);
    */
    
    // Calculated during complie time
    const float HALF_LN_2PI = 0.5 * log(2 * PI);
    const float DIV_LOG2_E = 1.0 / log2(EULER);     // To convert log2 => ln
    const float ONE_THREE_HUNDREDTHS = 1.0 / 300;   // Quicker than calculating 1 / (300 * x)

    const float COMMON = number - 0.2f;
    
    // Based on Stirling's approximation
    return  HALF_LN_2PI +
            (number - 0.5f) * fastLog2Bits(COMMON) * DIV_LOG2_E -
            COMMON +
            ONE_THREE_HUNDREDTHS / (number - 1);
    
    //return 0.673f * (number + 0.108f) * (number + 0.108f) + 0.71f * (number + 0.108f) + 0.0687f ;  // for x < 0.4, (~1% error)
}

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



// NOTE: Values very close to zero produce massive errors (technically correct but misleading?)
float percentError(const float exact, const float estimate) {
    return 100 * (estimate - exact) / ( (exact > estimate)? exact : estimate );
}


int main() {
    printf("Float exponent bias: %u\n", FLT_EXP_BIAS);
    printf("Double exponent bias: %u\n", DBL_EXP_BIAS);
    printf("Long Double exponent bias: %u\n\n", LDBL_MAX_EXP-1);

    // Comparison between approximation and true value
    printf("\n\n");
    for (float i=0; i<20; i+=0.1)
        printf("%3.2f:\t%.4f vs %.4f    (%+.4f%%)\n", i, log2f(i), fastLog2Alt(i),
                                                        percentError( log2f(i), fastLog2Alt(i) ));
    
    /*
    const float power = (float)1.001;
    for (float i=0; i<40; i+=0.1)
        printf("%3.2f:\t%.4f vs %.4f    (%+.4f%%)\n", i, powf(i, power), fastPow(i, power),
                                                        percentError( powf(i, power), fastPow(i, power) ) );
    */
    /*
    const float term_b = 32;
    for (float i=-10; i<10; i+=0.1)
        printf("%3.2f:\t%.4f vs %.4f    (%+.4f%%)\n", i, i / term_b, fastDivision(i, term_b),
                                                        percentError( i / term_b, fastDivision(i, term_b) ));
    */
    /*
    // Performance messuring
    size_t totalFloats = 10000;
    float rndFloats[totalFloats];
    for (size_t i=0; i < totalFloats; i++)
        rndFloats[i] = (float)rand() / rand();

    
    clock_t start_t = clock();

    float result;
    for (size_t i=0; i < 500000000; i++)
        result += fastReciprocal( rndFloats[i % totalFloats] );

    clock_t end_t = clock();

    printf("\nFloat: %.2f\n", result);
    printf("Start: %lu, End: %lu\n", start_t, end_t);
    printf("Execution time: %.2f\n", (float)(end_t - start_t) / CLOCKS_PER_SEC);
    */
}