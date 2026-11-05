# BlockType limb-processing benchmark

`cfloat` and `posit` take a BlockType template argument `bt`. They store and process a value as `nrBlocks = ceil(nbits / bits(bt))` limbs. These two benchmarks measure what that costs per operation when the same values are stored in `uint8_t`, `uint16_t`, `uint32_t` or `uint64_t` limbs (#1904).

| target | type | nbits |
|---|---|---|
| `benchmark_blocktype_cfloat_blocktype` | `cfloat<nbits, es, bt, true, false, false>`, es = 11, 15, 15, 19, 19 | 64, 96, 128, 196, 256 |
| `benchmark_blocktype_posit_blocktype` | `posit<nbits, 2, bt>` | 64, 96, 128, 196, 256 |

At one nbits, only the limb layout changes between the four rows. 196 is not a multiple of 16, 32 or 64, so every BlockType has a partially filled most significant limb at that width.

## Running

```bash
cmake -DCMAKE_BUILD_TYPE=Release -DUNIVERSAL_BUILD_BENCHMARK_PERFORMANCE=ON ..
make benchmark_blocktype_cfloat_blocktype benchmark_blocktype_posit_blocktype
taskset -c 2 ./benchmark/performance/arithmetic/benchmark_blocktype_cfloat_blocktype
```

- **Default run:** about 30 s per type, the median of 9 samples of at least 10 ms each.
- **Long mode:** configure with `-DUNIVERSAL_BUILD_REGRESSION_LEVEL_4=ON` (or `-DUNIVERSAL_BUILD_REGRESSION_STRESS=ON`), which defines `REGRESSION_LEVEL_4`, for 21 samples of at least 25 ms each.
- **`MANUAL_TESTING`:** times 128 bits only, as a quick look while working on the limb loops.

Pin the process to one core: timings vary by core on a hybrid processor.

## Method

The details are in `blocktype_harness.hpp`:

- **Operands:** 512 values per operand, built at run time from a seeded `mt19937_64`. Each value is a sum of overlapping random doubles, so the significand is random across its full width, and every row at one nbits gets the same values.
- **No dead code:** every pass stores its results and then runs a compiler barrier on that memory. Comparisons accumulate into a volatile sink.
- **Figures:** each one is the median ns/op over the samples. The second table divides each row by the `uint64_t` row at the same nbits.
- **Cross-check:** the results on the first 64 values are digested from their bit patterns and have to be identical for every BlockType at one nbits. A mismatch fails the run. All 40 configurations pass with both compilers.

## Reading (i7-12700K, gcc 15.2.0 and clang 21.1.8, Release, pinned to one core)

**cfloat**

- **Narrow limbs lose on `+`, `-` and `<`:**
  - At 64 bits, `uint8_t` is 10x to 12x slower than `uint64_t` with gcc and 22x with clang, and `uint16_t`/`uint32_t` are 2x to 3.5x slower.
  - At 96 to 256 bits, `uint8_t` is 1.3x to 7x slower and `uint16_t` 1.0x to 6x, depending on width and compiler.
- **`*`, `/` and `sqrt` care much less:**
  - `uint8_t` is 1.1x to 2.5x slower for `*` (5x with clang at 64 bits), 1.1x to 1.4x for `/` and 1.2x to 1.7x for `sqrt`.
  - `uint32_t` multiplies 3% to 11% faster than `uint64_t` at 96 to 256 bits with gcc.
- **`uint32_t` vs `uint64_t`:**
  - From 96 bits up, `uint32_t` runs at 0.8x to 1.45x the `uint64_t` time for every arithmetic operation.
  - At 64 bits, a single `uint64_t` limb is 2.5x faster for `+`/`-`.
  - The exception in these tables is conversion from `double`, where `uint64_t` is 8x to 70x slower: the fraction was copied bit by bit. Fixed by #1918 (see Follow-ups).
- **The cost doesn't grow smoothly with the limb count:**
  - With gcc, a 196-bit add costs 250 to 330 ns for every BlockType, while a 256-bit add costs 65 to 76 ns with `uint32_t`/`uint64_t`. Fixed by #1919 (see Follow-ups).
  - With clang, `uint8_t` add is about 4.5x slower than `uint16_t` at 96 and 128 bits.
  - The profile shows bit-serial loops in `normalizeAddition()`.
- **`<` costs as much as `-`:** with subnormals enabled, it is computed as a subtraction.
- **Copy** is 0.2 to 0.9 ns at every width. It is not a factor.
- **The old observation is not reproduced:** the previous cfloat benchmark showed `cfloat<16,5,uint32_t>` copy as slower than `uint8_t`. Measured again with this harness (an ad-hoc run, not in the grid), `uint32_t` copies in 0.16 ns against 0.51 ns for `uint8_t` with gcc, and all four are equal with clang. The old figure was the eliminated workload that benchmark's own comment called "suspect".

**posit**

- **`+`, `-`, `*` and `/` hardly depend on the BlockType:**
  - The ratio is 0.93x to 1.3x with gcc. With clang it is 0.85x to 1.05x from 96 bits up.
  - At 96 to 256 bits, `uint32_t` is faster than `uint64_t` for `+`, `-`, `*` and `/`: by 1% to 6% with gcc and 9% to 16% with clang.
  - `uint64_t` wins only at 64 bits (a single limb): 1.3x with gcc and 1.8x to 1.9x with clang.
- **The time is a fixed cost, not limb processing:**
  - Wide posit arithmetic costs 420 to 2670 ns per operation.
  - With `uint64_t` limbs, a posit add costs 15x to 35x a `cfloat` add of the same width. The exception is 196 bits with gcc (5.6x), where the `cfloat` add is itself slow (#1919).
  - On the same limbs, a posit multiply costs 3x to 7x a `cfloat` multiply, and a divide 1.8x to 2.6x.
  - `posit` `sqrt`, which runs on the limb kernels, takes 34 to 190 ns: 4x to 26x *less* than a posit add in the same configuration.
- **Conversion from `double`:** 3 to 5 ns at 64 bits, then 360 to 840 ns from 96 bits up, for every BlockType.
- **`<` is limb-bound:** 1 to 2.2 ns with `uint32_t`/`uint64_t`. `uint8_t` is 2.5x to 10x slower with gcc, and 2x to 6x with clang.

**Conclusion:**
- For `cfloat`, `uint32_t` and `uint64_t` are close, and `uint8_t`/`uint16_t` cost up to 7x on add, subtract and compare (22x at 64 bits with clang).
- For `posit`, the BlockType barely matters until the fixed cost of its arithmetic is removed.
- This benchmark makes no change to any default.

## Follow-ups

Slow paths these numbers expose, filed as issues:

1. #1918 (fixed): `cfloat` conversion from `double` copied the fraction one bit at a time with `uint64_t` limbs (`cfloat_impl.hpp`, the `bitsInBlock == 64` branch added for #1518). It now shifts the fraction into the at most two limbs it lands in. With `uint64_t` limbs at 96, 128, 196 and 256 bits, conversion now takes 1.4, 1.4, 20.5 and 4.5 ns with gcc, and 1.7, 1.6, 2.0 and 1.7 ns with clang, down from 130 to 150 ns. That is no slower than `uint32_t` except at 196 bits with gcc (1.1x). The tables below are the baseline from before the fix.
2. #1919 (fixed): `cfloat` add/sub: bit-serial loops make 196 bits cost 3.8x as much as 256 bits with gcc. The loops were in the exponent field: `exponent()` and `setexponent()` moved it one bit at a time whenever it straddles limbs, and `isnormal()`, `isdenormal()` and `scale()` call them several times per operation. Both now gather or scatter the field a limb at a time.
   - With gcc, a 196-bit add costs 50 ns with `uint64_t` limbs (was 248) and 72 ns with `uint32_t` (was 258), against 57 and 69 ns at 256 bits.
   - With clang, a 128-bit `uint8_t` add costs 91 ns (was 217).
   - `uint8_t`/`uint16_t` add at 196 and 256 bits is 2.1x to 3.3x faster with both compilers.
   - Multiply is 1.4x to 2.4x faster and divide 1.1x to 1.3x wherever the field straddles limbs.
   - Still slow: 64-bit `uint8_t` add, at 141 ns with gcc and 330 ns with clang. That is a different cost.
   - The tables below are the baseline from before the fix.
3. #1920 (fixed): `cfloat` `operator<` computed a full subtraction when `hasSubnormals` is set. It now compares the encodings limb by limb from the MSU down, after the NaN and zero checks, reversed for negatives.
   - With gcc, `lt` costs 1.0 to 2.1 ns at every width and BlockType (was 13 to 440).
   - With clang, it costs 1.2 to 3.3 ns (was 17 to 353).
   - The tables below are the baseline from before the fix.
4. #1921: `posit` add/mul/div and conversion from `double` above 64 bits carry a fixed cost of hundreds of ns that doesn't depend on the BlockType.

## Results

### cfloat, gcc 15.2.0

ns/op:

```text
nbits bt              add       sub       mul       div        lt      copy   fromdbl     todbl      sqrt
   64 uint8_t      136.51    142.26    166.03    255.97    145.03      0.51      1.94    156.15    167.41
   64 uint16_t      41.15     41.28     76.23    210.97     44.02      0.21      0.83    162.37    164.49
   64 uint32_t      32.21     36.05     81.30    206.48     37.80      0.16      0.42    173.87    161.79
   64 uint64_t      12.19     12.21     66.90    218.39     13.13      0.19      0.27    146.90    126.47
   96 uint8_t       85.04     87.70    133.44    363.96     88.82      0.61      5.77    257.21   5167.61
   96 uint16_t      63.45     65.06    102.03    301.21     63.73      0.24      4.93    275.55   4224.25
   96 uint32_t      51.65     51.82    111.41    298.67     53.01      0.24      4.32    260.82   3835.84
   96 uint64_t      45.58     48.69    117.00    323.48     49.75      0.21    132.99    241.16   3943.42
  128 uint8_t       94.90     95.98    170.05    532.03     97.42      0.52      6.55    360.56   7661.79
  128 uint16_t      76.18     84.39    126.18    467.54     72.67      0.17      2.39    380.53   6248.02
  128 uint32_t      60.80     63.91    134.85    438.84     62.11      0.16      4.50    347.66   5610.50
  128 uint64_t      42.30     46.04    151.67    467.94     47.05      0.22    132.57    340.57   5929.68
  196 uint8_t      332.19    349.30    437.73   1137.43    383.57      0.64     32.58    621.74  15557.05
  196 uint16_t     299.13    316.97    427.20   1053.86    353.75      0.38     21.18    606.46  13969.12
  196 uint32_t     257.58    281.16    395.20   1023.42    321.82      0.35     18.51    615.56  11159.91
  196 uint64_t     247.78    264.87    409.61   1025.26    300.63      0.29    151.84    597.92  11358.99
  256 uint8_t      381.19    402.40    535.46   1509.60    440.25      0.63     39.31    825.90  22244.93
  256 uint16_t     326.51    347.37    497.60   1390.78    378.32      0.25     22.95    803.99  19581.31
  256 uint32_t      76.25     77.59    285.55   1134.12     77.51      0.25      4.50    754.82  14384.31
  256 uint64_t      65.39     66.08    299.42   1182.11     64.18      0.29    131.94    753.50  13665.07
```

ratio to the `uint64_t` row at the same nbits:

```text
nbits bt              add       sub       mul       div        lt      copy   fromdbl     todbl      sqrt
   64 uint8_t       11.20     11.65      2.48      1.17     11.04      2.65      7.06      1.06      1.32
   64 uint16_t       3.38      3.38      1.14      0.97      3.35      1.11      3.04      1.11      1.30
   64 uint32_t       2.64      2.95      1.22      0.95      2.88      0.81      1.54      1.18      1.28
   64 uint64_t       1.00      1.00      1.00      1.00      1.00      1.00      1.00      1.00      1.00
   96 uint8_t        1.87      1.80      1.14      1.13      1.79      2.92      0.04      1.07      1.31
   96 uint16_t       1.39      1.34      0.87      0.93      1.28      1.15      0.04      1.14      1.07
   96 uint32_t       1.13      1.06      0.95      0.92      1.07      1.15      0.03      1.08      0.97
   96 uint64_t       1.00      1.00      1.00      1.00      1.00      1.00      1.00      1.00      1.00
  128 uint8_t        2.24      2.08      1.12      1.14      2.07      2.34      0.05      1.06      1.29
  128 uint16_t       1.80      1.83      0.83      1.00      1.54      0.77      0.02      1.12      1.05
  128 uint32_t       1.44      1.39      0.89      0.94      1.32      0.73      0.03      1.02      0.95
  128 uint64_t       1.00      1.00      1.00      1.00      1.00      1.00      1.00      1.00      1.00
  196 uint8_t        1.34      1.32      1.07      1.11      1.28      2.23      0.21      1.04      1.37
  196 uint16_t       1.21      1.20      1.04      1.03      1.18      1.32      0.14      1.01      1.23
  196 uint32_t       1.04      1.06      0.96      1.00      1.07      1.22      0.12      1.03      0.98
  196 uint64_t       1.00      1.00      1.00      1.00      1.00      1.00      1.00      1.00      1.00
  256 uint8_t        5.83      6.09      1.79      1.28      6.86      2.20      0.30      1.10      1.63
  256 uint16_t       4.99      5.26      1.66      1.18      5.89      0.89      0.17      1.07      1.43
  256 uint32_t       1.17      1.17      0.95      0.96      1.21      0.88      0.03      1.00      1.05
  256 uint64_t       1.00      1.00      1.00      1.00      1.00      1.00      1.00      1.00      1.00
```

### cfloat, clang 21.1.8

ns/op:

```text
nbits bt              add       sub       mul       div        lt      copy   fromdbl     todbl      sqrt
   64 uint8_t      350.38    351.54    380.97    238.84    352.76      0.51      1.46    115.91    115.77
   64 uint16_t      32.78     33.38     76.99    235.31     35.33      0.51      0.87     96.66    116.94
   64 uint32_t      31.21     31.68     65.76    227.98     33.35      0.51      0.70    117.75    116.43
   64 uint64_t      16.02     16.01     72.50    216.37     16.77      0.52      0.70     90.12     95.05
   96 uint8_t      187.10    201.35    216.81    471.73    230.27      0.63     28.01    264.27   6513.29
   96 uint16_t      42.56     43.17     88.56    324.38     44.15      0.63      7.03    205.84   3590.74
   96 uint32_t      38.00     38.27     92.97    314.45     39.58      0.64      4.51    212.87   3476.94
   96 uint64_t      41.05     41.32     98.36    333.22     42.25      0.54    144.47    214.61   3738.42
  128 uint8_t      216.95    231.03    244.01    599.20    260.10      0.52     28.09    376.89   9165.32
  128 uint16_t      47.61     47.98    111.72    442.55     49.26      0.52      7.03    316.10   5447.42
  128 uint32_t      41.69     42.03    122.29    442.61     43.43      0.55      6.71    320.14   5125.51
  128 uint64_t      41.43     41.74    138.25    456.85     43.06      0.52    122.91    314.64   5496.14
  196 uint8_t      266.35    281.59    362.53   1019.45    316.36      0.64     21.71    633.78  13325.36
  196 uint16_t     252.27    271.56    343.83    916.97    312.16      0.65     22.45    570.33  12430.44
  196 uint32_t      52.62     61.19    208.14    771.16     62.90      0.70     12.52    530.45   7937.36
  196 uint64_t      43.01     53.71    213.57    788.70     52.04      0.88    135.43    530.03   8733.87
  256 uint8_t      287.58    305.84    466.35   1378.80    344.13      0.76     21.36    769.22  19036.94
  256 uint16_t     279.76    299.62    423.98   1266.37    340.26      0.93     40.19    790.03  17000.26
  256 uint32_t      52.73     62.90    258.36   1091.97     61.87      0.76      1.98    715.19  12295.02
  256 uint64_t      66.67     67.15    294.65   1150.25     69.51      0.93    142.46    714.31  13032.37
```

ratio to the `uint64_t` row at the same nbits:

```text
nbits bt              add       sub       mul       div        lt      copy   fromdbl     todbl      sqrt
   64 uint8_t       21.87     21.96      5.25      1.10     21.03      0.99      2.09      1.29      1.22
   64 uint16_t       2.05      2.08      1.06      1.09      2.11      0.98      1.25      1.07      1.23
   64 uint32_t       1.95      1.98      0.91      1.05      1.99      0.99      1.00      1.31      1.22
   64 uint64_t       1.00      1.00      1.00      1.00      1.00      1.00      1.00      1.00      1.00
   96 uint8_t        4.56      4.87      2.20      1.42      5.45      1.18      0.19      1.23      1.74
   96 uint16_t       1.04      1.04      0.90      0.97      1.04      1.17      0.05      0.96      0.96
   96 uint32_t       0.93      0.93      0.95      0.94      0.94      1.19      0.03      0.99      0.93
   96 uint64_t       1.00      1.00      1.00      1.00      1.00      1.00      1.00      1.00      1.00
  128 uint8_t        5.24      5.54      1.76      1.31      6.04      1.00      0.23      1.20      1.67
  128 uint16_t       1.15      1.15      0.81      0.97      1.14      1.00      0.06      1.00      0.99
  128 uint32_t       1.01      1.01      0.88      0.97      1.01      1.06      0.05      1.02      0.93
  128 uint64_t       1.00      1.00      1.00      1.00      1.00      1.00      1.00      1.00      1.00
  196 uint8_t        6.19      5.24      1.70      1.29      6.08      0.73      0.16      1.20      1.53
  196 uint16_t       5.87      5.06      1.61      1.16      6.00      0.73      0.17      1.08      1.42
  196 uint32_t       1.22      1.14      0.97      0.98      1.21      0.80      0.09      1.00      0.91
  196 uint64_t       1.00      1.00      1.00      1.00      1.00      1.00      1.00      1.00      1.00
  256 uint8_t        4.31      4.55      1.58      1.20      4.95      0.81      0.15      1.08      1.46
  256 uint16_t       4.20      4.46      1.44      1.10      4.90      1.00      0.28      1.11      1.30
  256 uint32_t       0.79      0.94      0.88      0.95      0.89      0.82      0.01      1.00      0.94
  256 uint64_t       1.00      1.00      1.00      1.00      1.00      1.00      1.00      1.00      1.00
```

### posit, gcc 15.2.0

ns/op:

```text
nbits bt              add       sub       mul       div        lt      copy   fromdbl     todbl      sqrt
   64 uint8_t      557.81    559.65    523.34    598.26      2.43      0.51      3.45    258.75     76.80
   64 uint16_t     547.51    550.32    527.57    558.12      1.36      0.16      3.53    246.37     75.90
   64 uint32_t     551.15    553.30    534.76    550.50      1.00      0.15      3.32    263.63     75.06
   64 uint64_t     430.29    431.84    418.17    452.33      0.76      0.20      3.23    195.69     71.85
   96 uint8_t      717.08    723.55    687.18    910.57      3.27      0.61    387.06    298.57     55.02
   96 uint16_t     693.76    694.63    671.88    855.86      1.55      0.23    390.43    294.70     49.68
   96 uint32_t     688.45    690.70    671.22    833.17      1.37      0.23    397.75    293.65     43.75
   96 uint64_t     700.78    699.81    675.48    841.11      1.20      0.20    386.73    298.98     44.22
  128 uint8_t      947.20    960.44    920.20   1262.52      4.57      0.52    476.11    340.94     50.09
  128 uint16_t     893.53    894.63    874.36   1159.72      1.93      0.22    477.99    327.73     51.99
  128 uint32_t     895.39    893.53    875.03   1117.35      1.07      0.16    459.62    331.77     47.53
  128 uint64_t     918.58    919.21    911.85   1145.41      1.00      0.21    486.65    310.95     44.60
  196 uint8_t     1377.95   1400.37   1374.58   2000.28     14.51      0.65    659.70    413.01    123.36
  196 uint16_t    1330.04   1348.41   1312.83   1894.01      3.51      0.37    627.04    440.75     89.66
  196 uint32_t    1321.50   1321.65   1322.41   1843.90      1.57      0.36    630.59    429.57    100.40
  196 uint64_t    1389.99   1391.05   1379.13   1887.45      1.60      0.29    656.53    403.83     84.41
  256 uint8_t     1751.45   1767.45   1783.67   2669.69     15.34      0.64    809.66    489.52    133.14
  256 uint16_t    1669.99   1676.93   1694.64   2490.68      4.07      0.26    786.82    490.56     95.41
  256 uint32_t    1686.39   1688.96   1687.53   2419.64      1.64      0.26    784.42    503.95     94.94
  256 uint64_t    1794.11   1796.16   1795.12   2500.94      1.30      0.29    823.71    480.55     88.11
```

ratio to the `uint64_t` row at the same nbits:

```text
nbits bt              add       sub       mul       div        lt      copy   fromdbl     todbl      sqrt
   64 uint8_t        1.30      1.30      1.25      1.32      3.20      2.61      1.07      1.32      1.07
   64 uint16_t       1.27      1.27      1.26      1.23      1.79      0.82      1.09      1.26      1.06
   64 uint32_t       1.28      1.28      1.28      1.22      1.32      0.79      1.03      1.35      1.04
   64 uint64_t       1.00      1.00      1.00      1.00      1.00      1.00      1.00      1.00      1.00
   96 uint8_t        1.02      1.03      1.02      1.08      2.72      3.03      1.00      1.00      1.24
   96 uint16_t       0.99      0.99      0.99      1.02      1.29      1.15      1.01      0.99      1.12
   96 uint32_t       0.98      0.99      0.99      0.99      1.14      1.15      1.03      0.98      0.99
   96 uint64_t       1.00      1.00      1.00      1.00      1.00      1.00      1.00      1.00      1.00
  128 uint8_t        1.03      1.04      1.01      1.10      4.59      2.44      0.98      1.10      1.12
  128 uint16_t       0.97      0.97      0.96      1.01      1.93      1.02      0.98      1.05      1.17
  128 uint32_t       0.97      0.97      0.96      0.98      1.07      0.76      0.94      1.07      1.07
  128 uint64_t       1.00      1.00      1.00      1.00      1.00      1.00      1.00      1.00      1.00
  196 uint8_t        0.99      1.01      1.00      1.06      9.09      2.26      1.00      1.02      1.46
  196 uint16_t       0.96      0.97      0.95      1.00      2.20      1.29      0.96      1.09      1.06
  196 uint32_t       0.95      0.95      0.96      0.98      0.98      1.25      0.96      1.06      1.19
  196 uint64_t       1.00      1.00      1.00      1.00      1.00      1.00      1.00      1.00      1.00
  256 uint8_t        0.98      0.98      0.99      1.07     11.82      2.21      0.98      1.02      1.51
  256 uint16_t       0.93      0.93      0.94      1.00      3.13      0.90      0.96      1.02      1.08
  256 uint32_t       0.94      0.94      0.94      0.97      1.26      0.90      0.95      1.05      1.08
  256 uint64_t       1.00      1.00      1.00      1.00      1.00      1.00      1.00      1.00      1.00
```

### posit, clang 21.1.8

ns/op:

```text
nbits bt              add       sub       mul       div        lt      copy   fromdbl     todbl      sqrt
   64 uint8_t      775.30    790.94    746.95    579.81      2.12      0.52      4.54    250.66    185.39
   64 uint16_t     787.11    793.61    771.31    553.27      1.30      0.51      4.43    228.43    182.93
   64 uint32_t     805.21    801.65    765.81    542.82      1.23      0.51      4.19    254.69    190.06
   64 uint64_t     425.47    425.24    417.19    485.21      1.00      0.51      4.13    212.59     62.09
   96 uint8_t      632.08    635.97    583.98    846.06      3.22      0.62    371.59    267.48     35.24
   96 uint16_t     624.58    626.71    596.63    811.25      1.62      0.63    380.21    265.50     34.52
   96 uint32_t     596.67    599.75    579.55    771.91      1.08      0.63    359.79    283.69     34.00
   96 uint64_t     688.27    691.14    677.52    855.56      1.30      0.54    398.35    304.94     34.86
  128 uint8_t      822.93    830.20    792.57   1141.62      4.54      0.51    438.55    315.01     38.41
  128 uint16_t     805.37    810.23    772.97   1079.96      2.36      0.51    440.88    313.13     35.44
  128 uint32_t     781.39    781.86    762.97   1030.65      1.34      0.51    432.23    318.79     35.44
  128 uint64_t     901.67    901.21    894.75   1157.69      1.14      0.52    478.02    362.08     35.27
  196 uint8_t     1232.72   1252.51   1211.87   1902.23      5.35      0.65    607.64    376.40     76.82
  196 uint16_t    1201.94   1205.52   1171.54   1764.85      2.94      0.65    594.32    382.94     78.43
  196 uint32_t    1156.59   1156.29   1149.14   1673.51      1.63      0.64    591.68    386.01     75.29
  196 uint64_t    1315.09   1315.40   1309.66   1840.14      1.12      0.64    648.01    447.37     73.14
  256 uint8_t     1648.47   1634.32   1602.69   2547.84      7.34      0.65    759.80    444.32    100.13
  256 uint16_t    1545.54   1551.22   1515.41   2279.38      4.13      0.64    736.29    457.20     78.67
  256 uint32_t    1479.14   1482.13   1475.91   2224.33      2.17      0.64    721.00    456.94     79.26
  256 uint64_t    1685.18   1714.42   1763.45   2449.57      1.19      0.64    790.78    528.91     73.81
```

ratio to the `uint64_t` row at the same nbits:

```text
nbits bt              add       sub       mul       div        lt      copy   fromdbl     todbl      sqrt
   64 uint8_t        1.82      1.86      1.79      1.19      2.11      1.00      1.10      1.18      2.99
   64 uint16_t       1.85      1.87      1.85      1.14      1.30      0.99      1.07      1.07      2.95
   64 uint32_t       1.89      1.89      1.84      1.12      1.23      0.99      1.02      1.20      3.06
   64 uint64_t       1.00      1.00      1.00      1.00      1.00      1.00      1.00      1.00      1.00
   96 uint8_t        0.92      0.92      0.86      0.99      2.48      1.16      0.93      0.88      1.01
   96 uint16_t       0.91      0.91      0.88      0.95      1.25      1.17      0.95      0.87      0.99
   96 uint32_t       0.87      0.87      0.86      0.90      0.84      1.16      0.90      0.93      0.98
   96 uint64_t       1.00      1.00      1.00      1.00      1.00      1.00      1.00      1.00      1.00
  128 uint8_t        0.91      0.92      0.89      0.99      3.98      0.99      0.92      0.87      1.09
  128 uint16_t       0.89      0.90      0.86      0.93      2.07      0.98      0.92      0.86      1.01
  128 uint32_t       0.87      0.87      0.85      0.89      1.17      0.99      0.90      0.88      1.01
  128 uint64_t       1.00      1.00      1.00      1.00      1.00      1.00      1.00      1.00      1.00
  196 uint8_t        0.94      0.95      0.93      1.03      4.76      1.02      0.94      0.84      1.05
  196 uint16_t       0.91      0.92      0.89      0.96      2.62      1.02      0.92      0.86      1.07
  196 uint32_t       0.88      0.88      0.88      0.91      1.45      0.99      0.91      0.86      1.03
  196 uint64_t       1.00      1.00      1.00      1.00      1.00      1.00      1.00      1.00      1.00
  256 uint8_t        0.98      0.95      0.91      1.04      6.15      1.01      0.96      0.84      1.36
  256 uint16_t       0.92      0.90      0.86      0.93      3.46      1.00      0.93      0.86      1.07
  256 uint32_t       0.88      0.86      0.84      0.91      1.82      1.00      0.91      0.86      1.07
  256 uint64_t       1.00      1.00      1.00      1.00      1.00      1.00      1.00      1.00      1.00
```
