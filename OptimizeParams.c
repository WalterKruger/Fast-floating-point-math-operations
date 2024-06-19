# include <stdio.h>
# include <float.h>         // Create magic numbers
# include <xmmintrin.h>     // SSE instructions
# include "intsizedfloat.h" // To bit-shift a float
# include <time.h>          // Performance messuring

# include <math.h>          // For comparasion to approximation


float param_1, param_2, param_3, param_4;


float exp2_asm_alt_fast(const float input) {
    // Expontent only has 8-bits, but conversion is faster
    const intFlt_t int_part = input;

    union { uintFlt_t bit_repr; float as_value; } inp;
    inp.bit_repr = (int_part + 127) << (FLT_MANT_DIG - 1);

    // Taylor series w/t 4 terms: 2^x - 1 = ∑ [(x*ln⁡2)^n / n!]
    // Horner's method
    /*
    const float frac_x_ln2 = (input - int_part) * logf(2);
    const float frac_result = frac_x_ln2 * (
                                1.0f + frac_x_ln2 * (
                                    0.5f + frac_x_ln2 * (
                                        1.0f/6 + (1.0f / 24) * 
                                            frac_x_ln2)));
    */
    // Positive inputs (~0.004%), Negative inputs (~1.9%);
    const float frac_inp = input - int_part;
    const float frac_result = param_1 / (param_2 - frac_inp)
                                - param_3 * frac_inp
                                - param_4;
    
    return inp.as_value * (frac_result + 1);
}



float percentError(const float exact, const float estimate) {
    return 100 * (estimate - exact) / ( (exact > estimate)? exact : estimate );
}

int main() {
    
    float *p1_ptr = &param_1, *p2_ptr = &param_2, *p3_ptr = &param_3, *p4_ptr = &param_4;
    float p1_best, p2_best, p3_best, p4_best;
    float min_error = FLT_MAX;
    
    // Values that are close to optimal
    float   p1_inital = 27.567421,
            p2_inital = 4.957137,
            p3_inital = 0.434025,
            p4_inital = 5.561503;
    
    // Range to search from current
    const float DIFF = 0.01;
    const float RESOLUTION = 0.001;

    // Sample features to check error with
    const float SAMPLE_STEPS = 0.01;
    const float RNG_MIN = -2, RNG_MAX = 0;


#define LUT_I  (int)((inp - RNG_MIN + SAMPLE_STEPS * 1.5) * (1.0f / SAMPLE_STEPS))
    
    // Create lookup table as inputs will be the same for all params
    float EXACT_LUT[(int)((RNG_MAX - RNG_MIN) * (1.0 / SAMPLE_STEPS) + 1)];
    for (float inp = RNG_MIN; inp < RNG_MAX; inp += SAMPLE_STEPS) {
        EXACT_LUT[LUT_I] = exp2f(inp);
        //printf("Inp: %.4f, Index: %d, value: %f\n", inp, LUT_I, EXACT_LUT[LUT_I]);
    }

    #if 1

    for (*p1_ptr = p1_inital - DIFF; *p1_ptr < p1_inital + DIFF; *p1_ptr += RESOLUTION) {
    for (*p2_ptr = p2_inital - DIFF; *p2_ptr < p2_inital + DIFF; *p2_ptr += RESOLUTION) {
    for (*p3_ptr = p3_inital - DIFF; *p3_ptr < p3_inital + DIFF; *p3_ptr += RESOLUTION) {
    for (*p4_ptr = p4_inital - DIFF; *p4_ptr < p4_inital + DIFF; *p4_ptr += RESOLUTION) {
        // Sample vs real values and find max error for these params
        float max_error_in_sample = 0;

        for (float inp = RNG_MIN; inp < RNG_MAX; inp += SAMPLE_STEPS) {
            float percentage_error = fabsf( percentError(EXACT_LUT[LUT_I], exp2_asm_alt_fast(inp)) );

            if (percentage_error > max_error_in_sample)
                max_error_in_sample = percentage_error;
        }
        // If we found a smaller max error, save params
        if (max_error_in_sample < min_error) {
            min_error = max_error_in_sample;

            p1_best = param_1;
            p2_best = param_2;
            p3_best = param_3;
            p4_best = param_4;

            printf("\n\tError: %.6f%%\n", min_error);
            printf("\tBest params: %f, %f, %f, %f\n", p1_best, p2_best, p3_best, p4_best);
        }
    }}}
        //printf("\tBest error: %.4f%%\n", min_error);
    }

    printf("\n\nError: %f%%\n", min_error);
    printf("Best params: \n\t%f \n\t%f \n\t%f \n\t%f\n", p1_best, p2_best, p3_best, p4_best);
    
   #else
   *p1_ptr = p1_inital; *p2_ptr = p2_inital; *p3_ptr = p3_inital; *p4_ptr = p4_inital;

    float max_error_in_sample = 0;

    for (float inp = 2; inp < 3; inp += SAMPLE_STEPS) {
        float percentage_error = fabsf( percentError(EXACT_LUT[LUT_I], exp2_asm_alt_fast(inp)) );
        printf("%3.2f \t%.4f vs %.4f\t(Error: %+.4f%%)\n", inp, EXACT_LUT[LUT_I], exp2_asm_alt_fast(inp), percentage_error);

        if (percentage_error > max_error_in_sample)
            max_error_in_sample = percentage_error;
        }
    
    printf("Max error: %.4f", max_error_in_sample);

    #endif
}