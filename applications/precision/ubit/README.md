# Numerically challenging problems


Here's a summary of what each demonstrates:

```text
  ┌───────────────────┬──────────────────────────────────────────────┬───────────────────────────────────────────────────────────────────────┐
  │      Example      │                IEEE Behavior                 │                            areal Behavior                             │
  ├───────────────────┼──────────────────────────────────────────────┼───────────────────────────────────────────────────────────────────────┤
  │ rump.cpp          │ All IEEE types give ~10²¹ (wrong)            │ td_cascade (159 bits) needed for correct -0.827; areal flags overflow │
  ├───────────────────┼──────────────────────────────────────────────┼───────────────────────────────────────────────────────────────────────┤
  │ muller.cpp        │ Converges to 100 (wrong - should be 6)       │ Flags [UNCERTAIN] at iteration 4, warning not to trust                │
  ├───────────────────┼──────────────────────────────────────────────┼───────────────────────────────────────────────────────────────────────┤
  │ chaotic_bank.cpp  │ Double goes negative at year 18 (impossible) │ Could warn of accumulating error                                      │
  ├───────────────────┼──────────────────────────────────────────────┼───────────────────────────────────────────────────────────────────────┤
  │ quadratic.cpp     │ Float loses discriminant precision silently  │ Flags [UNCERTAIN] when discriminant cancellation occurs               │
  ├───────────────────┼──────────────────────────────────────────────┼───────────────────────────────────────────────────────────────────────┤
  │ thin_triangle.cpp │ Float has 15% error in Heron's formula       │ Flags [UNCERTAIN] on (s-a) where cancellation occurs                  │
  ├───────────────────┼──────────────────────────────────────────────┼───────────────────────────────────────────────────────────────────────┤
  │ newton.cpp        │ Shows convergence without indication         │ Flags [UNCERTAIN] when rounding affects division                      │
  └───────────────────┴──────────────────────────────────────────────┴───────────────────────────────────────────────────────────────────────┘
```

The key insight from Muller's recurrence is particularly striking - IEEE confidently computes 100, but the correct limit is 6. The areal type starts
flagging [UNCERTAIN] at iteration 4, warning the programmer that something is wrong before the result becomes completely nonsensical.

The Rump polynomial now shows that:
  - IEEE float/double/long double: All wrong
  - dd_cascade (~106 bits): Still wrong
  - td_cascade (~159 bits): CORRECT
  - qd_cascade (~212 bits): CORRECT

This demonstrates the extreme precision requirements for this pathological example - roughly 140-150 bits are needed to get the correct answer.



## Flag or enclosure: the #1637 applications

A single ubit tile (`areal`, `poxel`) is a **flag**: it reports that a result is inexact, but the tile it returns need not contain the true value. A `tile_interval<Tile>` (`universal/utility/tile_interval.hpp`), a pair of tiles, is an **enclosure**: it always contains the true value, and it decides a sign only when it can prove it. Each application compares rounding formats, single tiles and tile intervals at equal storage (the ubit counts toward the width), and asserts the outcome.

| application | the question | rounding formats | tile interval |
|---|---|---|---|
| `rump_polynomial.cpp` | f(77617, 33096) = -0.8274 | float, double and posit<32,2> are wrong; posit even has the wrong sign | contains -0.8274; honestly undecidable at 32 and 64 bits, where more than 120 bits are needed |
| `muller_kahan.cpp` | the limit of x_{n+1} = 111 - (1130 - 3000/x_{n-1})/x_n, which is 6 | all settle on 100; so does the single poxel tile, though flagged | contains the exact iterate at every step, then widens instead of converging to 100 |
| `geometric_predicates.cpp` | the sign of det(M^k), positive for every k | double gets k = 1 right, then reports 0 from k = 5 (with fused multiply-add) or the wrong sign at k = 20 (without) | never asserts a wrong sign; 64-bit tiles prove det(M) > 0 |
| `bbp_tail.cpp` | is the BBP tail S = 9.1e-22 in (0, 1e-16)? | half flushes to 0; posit<16,2> rounds S up to minpos | areal<16,5> and poxel<16,2> prove S > 0 but cannot bound it below 1e-16 (smallest tiles end at 1.2e-7 and 2.2e-16); poxel<16,3>, the same 16 bits with more range, proves 0 < S < 1e-16 |
| `griewank_sign.cpp` | the sign of 1 - cos x cos y - (x^2+y^2)/4000 at x = y = 1e-8, which is +1e-16 | float, double and posit<32,2> round cos to 1 and report g < 0 | undecidable at 32 bits and in areal<64,11>; poxel<64,2>, with 58 fraction bits near 1, proves g > 0 |
| `quadratic_roots.cpp` (#1646) | bound both roots of 3x^2 + 100x + 2 by the quadratic formula, where a and b occur twice (the dependency problem) | the textbook small root loses most of its digits to cancellation: 4% off in half, more than 100% in posit<16,2>, 1.3e-14 in double | every enclosure verified exactly; the textbook r1 is 1365+ tiles wide against 3 for the stable form and 1 for the tightest; at 32 and 64 bits ULP-wide operands widen it 4 to 7 times more |

The expected outcomes were checked against exact arithmetic before the applications were written (#1637).
