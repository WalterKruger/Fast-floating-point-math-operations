# Performance and error
Time taken to calculate 500 million results. Input was from an array of random valued floats (n=10k).
Error was messured as maximum percentage error.

The functions with the 'accurate' suffix and `fastPowViaLogf`  is from [Paul Mineiro](https://github.com/romeric/fastapprox) or [Ger Hobbelt](https://github.com/GerHobbelt/fastapprox).

'Perf' is in seconds. Using GCC at `-O3`, `-march=native` (Ryzen 5 5500 with FMA3) or the default.
### Exponentiation
Exponent: 1.45
| Function | Perf <sub>(m=n)</sub> | Perf <sub>(m=x86-64)</sub> | Neg. error | Pos. error | Notes |
| - | - | - | :--- | :--- | - |
| powf | 42.32 | 91.75 | $`-0.00\%`$ | $`0.00\%`$ | glibc |
| fastPow | 0.45 | 0.50 | $`-7.05\%`$ | $`7.30\%`$ | 1 |
| fastPowAlt | 0.76 | 0.98 | $`-3.53\%`$ | $`3.54\%`$ | 1 |
| fastPowAltAlt | 0.90 | 1.02 | $`-2.95\%`$ | $`2.94\%`$ | |
| powViaLogf | 21.44 | 21.31 | $`-0.00\%`$ | $`0.00\%`$ | 2 |
| fastPowViaLogf | 1.95 | 2.25 | $`-0.0212\%`$ | $`0.0015\%`$ | 1 |
| fastPowViaLogf_A | 1.50 | 1.77 | $`-0.0189\%`$ | $`0.0050\%`$ | 1, 2 |

1. Accuracy scales with exponent
2. Fails with negative bases

### Binary logarithm
Error for inputs > 5.
| Function | Perf <sub>(m=n)</sub> | Perf <sub>(m=x86-64)</sub> | Neg. error | Pos. error | Notes | Error at 1 | Near 1 |
| - | - | - | :--- | :--- | - | - | - |
| log2f | 12.30 | 12.10 | $`-0.00\%`$ | $`0.00\%`$ | glibc |
| log2_fpu | 8.92 | 8.81 | $`-0.00\%`$ | $`0.00\%`$ | x87 |
| fastLog2 | 0.36 | 0.36 | $`-1.71\%`$ | $`1.43\%`$ | 1 | Awful | $`10\%`$ |
| fastLog2Alt 1 | 0.56 | 0.70 | $`-0.26\%`$ | $`0.22\%`$ | 1 | Awful | $`3\%`$ |
| fastLog2Alt 2 | 0.65 | 0.89 | $`-0.0543\%`$ | $`0.0612\%`$ | 1 | Awful | $`1.5\%`$ |
| fastLog2Alt 3 | 0.64 | 0.70 | $`-0.0514\%`$ | $`0.0562\%`$ | 1 | Awful | $`1.2\%`$ |
| fastLog2Bits | 0.59 | 0.63 | $`-0.26\%`$ | $`0.22\%`$ | 1 | Awful | $`3\%`$ |
| fastLog2_accurate | 0.56 | 0.65 | $`-0.0052\%`$ | $`0.0004\%`$ | 1 | $`0.3\%`$ | $`0.1\%`$ |

1. Ill-conditioned near 1. Gets more accurate the larger the input

### Binary exponent
| Function | Perf <sub>(m=n)</sub> | Perf <sub>(m=x86-64)</sub> | Neg. error | Pos. error | Notes |
| - | - | - | :--- | :--- | - |
| exp2f | 14.92 | 14.93 | $`-0.00\%`$ | $`0.00\%`$ | glibc |
| exp2_asm | 11.90 | 11.77 | $`-1.10`$E$`-05\%`$ | $`1.12`$E$`-05\%`$ | x87, 1 |
| exp2_asm_alt | 7.00 | 7.30 | $`-1.10`$E$`-05\%`$ | $`1.12`$E$`-05\%`$ | |
| fastExp2 | 0.36 | 0.37 | $`-2.94\%`$ | $`2.94\%`$ | |
| fastExp2_alt | 0.59 | 0.75 | $`-0.18\%`$ | $`0.18\%`$ | 2 |
| fastExp2_bits| 0.83 | 1.27 | $`-0.088\%`$ | $`1.11`$E$`-05\%`$ | 3 |
| fastExp2_acc. | 1.09 | 1.20 | $`-0.0066\%`$ | $`0.0023\%`$ | |
| fastExp2_acc._A | 0.88 | 0.95 | $`-0.0053\%`$ | $`0.0054\%`$ | |

1. Inputs $`\le`$ -1 don't work
2. Negative inputs don't work
3. Negative inputs are ~3.5x more inaccurate than listed
