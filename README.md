# Overview
A collection of functions that approximate math operations using floating-point hacks. 
These exploit a floating-point's binary representation in combination with logarithmic identities to avoid costly operations.

See `Deriving formulas.pdf` for an explanation for why these works.

Note: **Not all approximations will be much faster than C’s standard offerings**. 
The general power, log gamma, and binary log approximations are the only function substantially faster on my machine (Ryzen 5 5500), but it could be useful in embedded or RISC environments.

Based on the famous “Fast inverse square root” algorithm.

## Performance and error
Time taken to calculate 500 million results. Input was from an array of random valued floats (n=10k).
Error was messured as maximum percentage error.

Done with `-march=native` (FMA3).
### Exponentiation
Exponent: 1.45
| Function | Seconds | Neg. error | Pos. error | Notes |
| - | - | :--- | :--- | - |
| powf | 42.32 | $`-0.00\%`$ | $`0.00\%`$ | glibc |
| fastPow | 0.45 | $`-7.05\%`$ | $`6.31\%`$ | 1 |
| fastPowAlt | 0.76 | $`-3.10\%`$ | $`2.42\%`$ | 1 |
| fastPowAltAlt | 0.90 | $`-2.95\%`$ | $`2.92\%`$ | |
| powViaLogf | 21.44 | $`-0.00\%`$ | $`0.00\%`$ | 2 |
| fastPowViaLogf | 1.95 | $`-0.00155\%`$ | $`0.0009\%`$ | 1 |
| fastPowViaLogf_A | 1.50 | $`-0.0142\%`$ | $`0.0059\%`$ | 1, 2 |

1. Accuracy scales with exponent
2. Fails with negative bases

### Binary logarithm
Error for inputs > 5.
| Function | Seconds | Neg. error | Pos. error | Notes | Error at 1 | Near 1 |
| - | - | :--- | :--- | - | - | - |
| log2f | 12.30 | $`-0.00\%`$ | $`0.00\%`$ | glibc |
| log2_fpu | 8.92 | $`-0.00\%`$ | $`0.00\%`$ | x87 |
| fastLog2 | 0.36 | $`-1.71\%`$ | $`1.06\%`$ | 1 | Awful | $`10\%`$ |
| fastLog2Alt 1 | 0.56 | $`-0.23\%`$ | $`0.22\%`$ | 1 | Awful | $`3\%`$ |
| fastLog2Alt 2 | 0.65 | $`-0.0543\%`$ | $`0.0610\%`$ | 1 | Awful | $`1.5\%`$ |
| fastLog2Alt 3 | 0.64 | $`-0.0697\%`$ | $`0.0561\%`$ | 1 | Awful | $`1.2\%`$ |
| fastLog2Bits | 0.59 | $`-0.23\%`$ | $`0.22\%`$ | 1 | Awful | $`3\%`$ |
| fastLog2_accurate | 0.56 | $`-0.0035\%`$ | $`0.0003\%`$ | 1 | $`0.3\%`$ | $`0.1\%`$ |

1. Ill-conditioned near 1. Gets more accurate the larger the input

### Binary exponent
| Function | Seconds | Neg. error | Pos. error | Notes |
| - | - | :--- | :--- | - |
| exp2f | 14.92 | $`-0.00\%`$ | $`0.00\%`$ | glibc |
| exp2_asm | 11.90 | $`-7.00`$E$`-06\%`$ | $`7.00`$E$`-06\%`$ | x87, 1 |
| exp2_asm_alt | 7.00 | $`-9.03`$E$`-06\%`$ | $`9.03`$E$`-06\%`$ | |
| fastExp2 | 0.36 | $`-2.93\%`$ | $`2.94\%`$ | |
| fastExp2_alt | 0.59 | $`-0.18\%`$ | $`0.18\%`$ | 2 |
| fastExp2_bits| 0.83 | $`-0.085\%`$ | $`1.20`$E$`-05\%`$ | 3 |
| fastExp2_acc. | 1.09 | $`-0.0066\%`$ | $`0.0017\%`$ | |
| fastExp2_acc._A | 0.88 | $`-0.0038\%`$ | $`0.0049\%`$ | |

1. Inputs $`\le`$ -1 don't work
2. Negative inputs don't work
3. Negative inputs are ~3.5x more inaccurate than listed

## Included operations
| Operation  | Fast approximation | Standard equivalent |
| - | - | - |
| $`\log_2{}`$  | `fastLog2`, `fastLog2Bits`, `fastLog2Alt`  | `log2f` |
| $`\sqrt{n}`$ | `fastSqrRoot` `sqrRootSSE` | `sqrtf` |
| $`\frac{1}{\sqrt{n}}`$  | `fastInvSqrRoot`, `fastInvSqrRoot_DBL`, `invSqrRootSSE`  | `1.0f / sqrtf` |
| General power | `fastPow`, `fastPowAlt` | `powf` |
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
