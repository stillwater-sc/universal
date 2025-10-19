# 8-4 Compression Algorithm

## Expansion Algebra Education Summary

### Core Principle: Error-Free Transformations

  Expansion algebra is built on error-free transformations like two_sum and fast_two_sum:

```cpp
  fast_two_sum(a, b, sum, error)
  // sum = a + b (rounded)
  // error = (a + b) - sum (the bits that were lost to rounding)
  // Guarantee: a + b = sum + error EXACTLY
```

### The 8→4 Compression Algorithm

  The basic algorithm has two phases:

   - Phase 1: Bottom-Up Accumulation

  Starting from the least significant components, accumulate upward using error-free operations:

```cpp
  fast_two_sum(r6, r7, s0, r7);  // s0 = r6+r7, error stored back in r7
  fast_two_sum(r5, s0, s0, r6);  // s0 = r5+s0, error stored back in r6
  fast_two_sum(r4, s0, s0, r5);  // Continue pattern...
  fast_two_sum(r3, s0, s0, r4);
  fast_two_sum(r2, s0, s0, r3);
  fast_two_sum(r1, s0, s0, r2);
  fast_two_sum(r0, s0, r0, r1);  // Final: r0 = r0+s0, error in r1
```

  Why bottom-up? Because smaller values are added to larger ones, minimizing lost precision.

   - Phase 2: Conditional Extraction

  Extract the 4 most significant non-overlapping components using conditional logic to handle zeros:

```cpp
  fast_two_sum(r0, r1, s0, s1);
  if (s1 != 0.0) {
      fast_two_sum(s1, r2, s1, s2);
      if (s2 != 0.0) {
          // ... continue extracting non-zero components
      }
  }
```

  Why conditional? Components might cancel to zero. The algorithm dynamically shifts remaining precision into available slots.

### What Was Wrong

The original bug (line 195 of qd_cascade_impl.hpp):

```cpp
compressed[3] = result[3] + result[4] + result[5] + result[6] + result[7];  // WRONG!
```

This naive addition discards rounding errors from intermediate sums, destroying the 212-bit precision of quad-double arithmetic.

### The Fix

We added compress_8to4() to floatcascade.hpp that implements the proven QD library algorithm, then used it in qd_cascade::operator+= and operator-=:

```cpp
  auto result = expansion_ops::add_cascades(cascade, neg_rhs);  // 8 components
  cascade = expansion_ops::compress_8to4(result);  // Properly compress to 4
```
