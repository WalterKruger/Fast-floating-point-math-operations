#ifndef INTSIZEDFLOAT_H_    /* Guard to prevent double def */
# define INTSIZEDFLOAT_H_

#include <float.h>  // To get the float sizes
#include <stdint.h> // Ints with an exact size

/* C doesn't specify the exact sizes of floating point types, but do have minimum range/precision requirements.
**
** === From "ISO/IEC 9899" (Programming languages — C) ===
** A 'float' must be able to represent numbers from {1E-37, 1E+37}                  (5.2.4.2.2 12&13)
** Minimum difference between 1 and the least value greater than 1 (EPSILON): 1E-5  (5.2.4.2.2 13)
** 'float' values are a sub-set of 'double', 'double' is sub-set of 'long double'   (6.2.5 10)
*/

// Integer type for a 'float'
#if FLT_MANT_DIG == 24
    typedef uint32_t uintFlt_t;
    typedef int32_t intFlt_t;
#elif FLT_MANT_DIG == 53
    typedef uint64_t intFlt_t;
#else
    #error "Unsupported float size"
#endif

// Integer type for a 'double'
#if DBL_MANT_DIG == 24
    typedef uint32_t uintDbl_t;
    typedef int32_t intDbl_t;
#elif DBL_MANT_DIG == 53
typedef uint64_t uintDbl_t;
    typedef int64_t intDbl_t;
#elif defined(__SIZEOF_INT128__) && DBL_MANT_DIG <= 113
    typedef __uint128_t uintDbl_t;
    typedef __int128_t intDbl_t;
#else
    #error "Unsupported double size"
#endif

// Integer type for a 'long double'
#if LDBL_MANT_DIG == 24
    typedef uint32_t uintLDbl_t;
    typedef int32_t intLDbl_t;
#elif LDBL_MANT_DIG == 53
    typedef uint64_t uintLDbl_t;
    typedef int64_t intLDbl_t;
#elif defined(__SIZEOF_INT128__) && LDBL_MANT_DIG <= 113
    typedef __uint128_t uintLDbl_t;
    typedef __int128_t intLDbl_t;
#else
    #error "Unsupported long double size"
#endif


// GCC's 128-bit float (compiler intrinsic)
#ifdef __SIZEOF_FLOAT128__
    #ifdef __SIZEOF_INT128__
        typedef __uint128_t __intFlt128_t;
    #else
        #error "Float128 available but no 128-bit int"
    #endif
#endif

#endif