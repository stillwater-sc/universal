# lindex: level-index regression tests

`lindex<nbits, lbits, bt>` stores a value X >= 0 as its generalized logarithm
psi(X) = l + f, a level l and an index f in [0, 1), with phi(x) = e^phi(x - 1) above 1
and phi(x) = x below. See [docs/number-systems/lindex.md](../../../docs/number-systems/lindex.md)
for the type and [docs/level-index-design.md](../../../docs/level-index-design.md) for the
design and its verification.

Every test compares **encodings** for equality against an mpmath oracle at 600 bits. None
goes through double, and none uses a round trip. An error is a distance in psi, in units of
2^-fbits, never a value-domain error: one psi-ulp at the top of level 4 spans a factor of
10^16568.

| directory | test | checks | oracle |
|---|---|---|---|
| `kernel/` | `kernel.cpp` | the shared phi/psi kernel (`number/shared/level_index_kernel.hpp`): 592 add/sub pairs per width, every level, error in psi-ulps, faithful and correctly rounded | `tools/generators/level_index_gen.py` |
| `api/` | `api.cpp` | triviality, special encodings in every block type, numeric_limits, text, fields | -- |
| `api/` | `core_only.cpp` | `core.hpp` alone compiles, links and runs, with no stream in the translation unit | -- |
| `api/` | `exceptions.cpp` | `LINDEX_THROW_ARITHMETIC_EXCEPTION`: x / 0 throws, nothing else does | -- |
| `conversion/` | `conversion.cpp` | double, float and 64-bit integers to encodings, and encodings to correctly rounded doubles and floats, in six configurations | `tools/generators/lindex_conversion_gen.py` |
| `logic/` | `logic.cpp` | comparison: every pair at 8 bits, sampled at 16, 32 and 64 | the decoded signed psi and the order of the doubles |
| `arithmetic/` | `arithmetic.cpp` | + - * /: every signed pair of `lindex<8,3>`, sampled 12 to 64 bits, the rounding-retry cases | `tools/generators/lindex_arithmetic_gen.py` |
| `math/` | `correct_rounding.cpp` | the math library: every `lindex<8,3>` encoding for 14 functions, every pair for pow and hypot, sampled 16 to 64 bits, and the exact exp/log shifts | `tools/generators/lindex_math_gen.py` |

The generators are checked in and do not run during the build; each reproduces its
vectors header byte for byte.

```text
cmake -DUNIVERSAL_BUILD_NUMBER_LINDEX=ON ..
cmake --build . --target lindex_arithmetic
./static/logarithmic/lindex/lindex_arithmetic
```

`education/tables/lindex.cpp` writes the encoding tables (`edu_tables_lindex`, add `-csv`
for CSV).
