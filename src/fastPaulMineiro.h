/*=====================================================================*
 *                   Copyright (C) 2011 Paul Mineiro                   *
 * All rights reserved.                                                *
 *                                                                     *
 * Redistribution and use in source and binary forms, with             *
 * or without modification, are permitted provided that the            *
 * following conditions are met:                                       *
 *                                                                     *
 *     * Redistributions of source code must retain the                *
 *     above copyright notice, this list of conditions and             *
 *     the following disclaimer.                                       *
 *                                                                     *
 *     * Redistributions in binary form must reproduce the             *
 *     above copyright notice, this list of conditions and             *
 *     the following disclaimer in the documentation and/or            *
 *     other materials provided with the distribution.                 *
 *                                                                     *
 *     * Neither the name of Paul Mineiro nor the names                *
 *     of other contributors may be used to endorse or promote         *
 *     products derived from this software without specific            *
 *     prior written permission.                                       *
 *                                                                     *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND              *
 * CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,         *
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES               *
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE             *
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER               *
 * OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,                 *
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES            *
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE           *
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR                *
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF          *
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT           *
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY              *
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE             *
 * POSSIBILITY OF SUCH DAMAGE.                                         *
 *                                                                     *
 * Contact: Paul Mineiro <paul@mineiro.com>                            *
 *=====================================================================*/

#ifndef FAST_PAUL_MINEIRO_H
#define FAST_PAUL_MINEIRO_H



// Paul Mineiro: https://github.com/romeric/fastapprox
float fastLog2_accurate(const float number) {
    union {float as_value; uintFlt_t bit_repr;} num, mant;
    num.as_value = number; mant.as_value = number;
    
    // Clear out non-mantissa bits & set exp range: (1, 0.5]
    mant.bit_repr &= FLT_BIT_TERM - 1;
    mant.bit_repr |= (FLT_EXP_BIAS - 1) << (FLT_MANT_DIG - 1);

    return (float)num.bit_repr * 1.1920928955078125e-7f
            - 124.22551499f
            - 1.498030302f * mant.as_value
            - 1.72587999f / (0.3520887068f + mant.as_value);
}

// Paul Mineiro http://www.machinedlearnings.com/2011/06/fast-approximate-logarithm-exponential.html
float fastExp2_accurate(float input) {
    
    const float UNDERFLOW_THRESHOLD = 2.0f - FLT_MAX_EXP;

    //if (input < UNDERFLOW_THRESHOLD) return 0;
    //if (input >= FLT_MAX_EXP) return INFINITY;

    //input = (input < UNDERFLOW_THRESHOLD)? UNDERFLOW_THRESHOLD : input;   // Prevent int underflow
    
    const float sign = (input < 0) ? 1.0f : 0.0f;

    const float frac_part = input - (int)input + sign;
    
    union { uintFlt_t bit_repr; float as_value; } result;
    
    result.bit_repr =   FLT_BIT_TERM * (
                            input + 121.2740575f
                            + 27.7280233f / (4.84252568f - frac_part)
                            - 1.49012907f * frac_part
                        );
    
    return result.as_value;
}

// Ger Hobbelt: https://github.com/GerHobbelt/fastapprox/blob/master/fastapprox/src/fastexp.h
// A bit faster, but less accurate + edge cases?
float fastExp2_accurate_A(const float input) {
    const uintFlt_t EXP_MASK = (uintFlt_t)0b1111111 << (FLT_MANT_DIG - 1);
    const uintFlt_t LOW_15_MASK = 0x7fff;
    const uintFlt_t MANT_MASK = FLT_BIT_TERM - 1;

    union {float as_value; uintFlt_t bit_repr;}
        result = {input + 383.f}, zv;

    zv.bit_repr = EXP_MASK | ((result.bit_repr & LOW_15_MASK) << 8);
    result.as_value =   result.as_value
                        - 4.23579f
                        + 27.7280f / (5.84252568f - zv.as_value)
                        - 1.49012907f * zv.as_value;

    //result.bit_repr = (result.bit_repr < 0x43808000)? 0 : result.bit_repr;    // Underflow protection
    result.bit_repr = (result.bit_repr &  MANT_MASK) << 8;
    
    return result.as_value;
}


__m128 fastExp2_acc_A_vec(const __m128 input) {
    const __m128i EXP_MASK = _mm_set1_epi32((uintFlt_t)0b1111111 << (FLT_MANT_DIG - 1));
    const __m128i LOW_15_MASK = _mm_set1_epi32(0x7fff);
    const __m128i MANT_MASK = _mm_set1_epi32(FLT_BIT_TERM - 1);

    union {__m128 as_value; __m128i bit_repr;} result, zv;
    result.as_value = _mm_add_ps(input, _mm_set1_ps(383.f));

    zv.bit_repr = _mm_and_si128(result.bit_repr, LOW_15_MASK);
    zv.bit_repr = _mm_slli_epi32(zv.bit_repr, 8);
    zv.bit_repr = _mm_and_si128(zv.bit_repr, EXP_MASK);


    result.as_value = _mm_sub_ps(result.as_value, _mm_set1_ps(4.23579f));    

    __m128 denom = _mm_sub_ps(_mm_set1_ps(5.84252568f), zv.as_value);
    __m128 frac = _mm_div_ps(_mm_set1_ps(27.7280f), denom);
    result.as_value = _mm_add_ps(result.as_value, frac);

    __m128 mulTerm = _mm_mul_ps(_mm_set1_ps(1.49012907f), zv.as_value);
    result.as_value = _mm_sub_ps(result.as_value, mulTerm);

    return result.as_value;
}

#endif