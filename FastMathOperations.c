# include <stdio.h>
# include <float.h>         // Create magic numbers
#include "src/floatConst.h"     // To make values based on float.h more readable
//# include <xmmintrin.h>     // SSE instructions
# include <time.h>          // Performance messuring
# include <math.h>          // For comparasion to approximation
# include <string.h>        // Memcpy 


// Math approximation libary functions
# include "src/fastLog2.h"
# include "src/fastLog.h"
# include "src/fastExp.h"
# include "src/fastExp2.h"
# include "src/fastSqrt.h"
# include "src/fastReciprocal.h"

# include "src/fastPow.h"
# include "src/fastArithmetic.h"
# include "src/fastPaulMineiro.h"   // Exp2 & log2




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
    return  HALF_LN_2PI
            + (number - 0.5f) * fastLog2_accurate(COMMON) * DIV_LOG2_E
            - COMMON
            + ONE_THREE_HUNDREDTHS / (number - 1);
    
    //return 0.673f * (number + 0.108f) * (number + 0.108f) + 0.71f * (number + 0.108f) + 0.0687f ;  // for x < 0.4, (~1% error)
}



float fastPowViaLogf_OWN(const float number, const float exponent) {
    return fastExp2_bits( exponent * fastLog2_accurate(number) );
}

// More accurate, but int exp only. Also accuracy scales heavily for exponent
// Fairly slow compared to others
float fastPow_TEST(const float number, const uint8_t exponent) {
    // Need to operate on bit representation
    union {float as_value; uintFlt_t bit_repr;} num;
    num.as_value = number;

    const float COMMON = FLT_BIT_TERM * (LOG_APPROX_CORRECT - FLT_EXP_BIAS);
    num.bit_repr = exponent * num.bit_repr + (exponent - 1) * COMMON;
    //num.bit_repr = exponent * (num.bit_repr + COMMON) - COMMON;   // Alternative form

    // Newton's method to improve guess (uses an exponential!)
    num.as_value *= 1 - exponent * (1 - number * fastInvRootApprox(num.as_value, exponent));

    return num.as_value;
}





// NOTE: Values very close to zero produce massive errors (technically correct but misleading?)
float percentError(const float exact, const float estimate) {
    return 100 * (estimate - exact) / ( (exact > estimate)? exact : estimate );
}

int main() {
    printf("Float exponent bias: %u\n", FLT_EXP_BIAS);
    printf("Double exponent bias: %u\n", DBL_EXP_BIAS);
    printf("Long Double exponent bias: %u\n\n", LDBL_MAX_EXP-1);

    #if 1
    // Comparison between approximation and true value
    #define PRINT_VALE 1
    # if PRINT_VALE == 1

        printf("\n\n");
        for (float i=0; i<20; i+=0.1)
            printf("%3.2f:\t%.4f vs %.4f    (%+.4f%%)\n", i, log2f(i), fastLog2(i),
                                                            percentError( log2f(i), fastLog2(i) ));
    # elif PRINT_VALE == 2
    
        float exponent = (float)1.45;
        for (float i=0; i<20; i+=0.1)
            printf("%3.2f:\t%.4f vs %.4f    (%+.4f%%)\n", i, powf(i, exponent), fastPowViaLogf_A(i, exponent),
                                                            percentError( powf(i, exponent), fastPowViaLogf_A(i, exponent) ) );
    # elif PRINT_VALE == 3
    
        const float term_b = 1;
        for (float i=-10; i<10; i+=0.1)
            printf("%3.2f:\t%.4f vs %.4f    (%+.4f%%)\n", i, i / term_b, fastDivision(i, term_b),
                                                            percentError( i / term_b, fastDivision(i, term_b) ));
    # elif PRINT_VALE == 4

        // Vector version
        printf("\n\n");
        float vec_exp = 1.45;
        for (float i=0; i<20; i+=0.1) {
            float result = _mm_cvtss_f32(fastPow_vec(_mm_set_ss(i), _mm_set_ss(vec_exp)));

            printf("%3.2f:\t%.4f vs %.4f    (%+.4f%%)\n", i,    powf(i, vec_exp), result,
                                                percentError(   powf(i, vec_exp), result ));
        }
    #endif
    
    #else
    
    // Performance messuring
    size_t totalFloats = 10000;
    float rndFloats[totalFloats];
    for (size_t i=0; i < totalFloats; i++)
        rndFloats[i] = (float)rand() / rand();
        //rndFloats[i] = (float)rand() / INT16_MAX;   // (0, 1)

    
    clock_t start_t = clock();

    float result;
    for (size_t i=0; i < 500000000; i++)
        result += fastPow(rndFloats[i % totalFloats], 1.45 );
        //printf("%.2f\t", rndFloats[i % totalFloats]);

    clock_t end_t = clock();

    printf("\nFloat: %.2f\n", result);
    printf("Start: %lu, End: %lu\n", start_t, end_t);
    printf("Execution time: %.2f\n", (float)(end_t - start_t) / CLOCKS_PER_SEC);  
    
    #endif
}