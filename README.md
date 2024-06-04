# Overview
A collection of functions that approximate math operations using floating-point hacks. 
These exploit a floating-point's binary representation in combination with logarithmic identities to avoid costly operations.

See `Deriving formulas.pdf` for an explanation for why these works.

Note: **Not all approximations will be much faster than C’s standard offerings**. 
The general power, log gamma, and binary log approximations are the only function substantially faster on my machine (Ryzen 5 5500), but it could be useful in embedded or RISC environments.

Based on the famous “Fast inverse square root” algorithm.

## Included operations
See also: [Performance and accuracy](perf_and_accuracy.md)

| Operation  | Fast approximation | Standard equivalent |
| - | - | - |
| $`\log_2{}`$  | `fastLog2`, `fastLog2Bits`, `fastLog2Alt`  | `log2f` |
| $`\sqrt{n}`$ | `fastSqrRoot` `sqrRootSSE` | `sqrtf` |
| $`\frac{1}{\sqrt{n}}`$  | `fastInvSqrRoot`, `fastInvSqrRoot_DBL`, `invSqrRootSSE`  | `1.0f / sqrtf` |
| General power | `fastPow`, `fastPowAlt`, `fastPowAltAlt` | `powf` |
| $`2^n`$ | `fastExp2`, `fastExp2_alt`, `fastExp2_bits` | `exp2f` |
| 1/n<sup>th</sup> root | `fastInvRootApprox` | `powf(n, 1.0f / root)` |
| $`\frac{1}{n}`$ | `fastReciprocal`, `reciprocalSSE` | `1.0f / n` |
| $`\ln{(\Gamma{(n)})}`$ | `fastLogGamma` | `lgammaf` |
| Multiplication | `fastMultiply` | `a * b` |
| Division | `fastDivision` | `a / b` |

## Portability
### Floating-point format
All method used only work for IEEE 754 binary types, or any format substantially similar. 
These approximation assumes a normalized input/output so they will become less accurate the smaller the input when in the sub-normal range.

Operations that are not defined for negative inputs will not return a NaN, so care should be taken to ensure valid inputs.

### Other IEEE floats
One of the goals on this project was ensuring portability across differently sized floating-point types.
Thus, I used the header file `intsizedfloat.h` so that in the rare case that the compiler uses a 64-bit float, the integer used to hold the float when type punning will match. 
For the vast majority of compilers, a uint32_t will work without issue.

Other floating-point types (double, quadruple, and half) should work so long as the macro constants are replaced with their associated values. 
`fastInvSqrRoot_DBL` shows and example for a C’s double.


## Note on type punning
In order to manipulate its binary representation, we reinterpret a float as an unsigned integer using a union. 
According to the C standard, this is defined behaviour so long as both types are the same size. 

However, **in C++ this is undefined behaviour**. Instead, use C++20’s `std::bit_cast` or `std::memcpy`. 
On modern C/C++ compilers, `memcpy()` is often implemented as a compiler intrinsic so a 32 or 64-bit load [will not produce calls to a function](https://godbolt.org/z/oxhvYTTav).

If you port this to another language, be careful to ensure your method of type punning is well defined.
