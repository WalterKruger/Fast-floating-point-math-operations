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
