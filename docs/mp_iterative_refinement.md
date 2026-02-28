# Mixed-precision Iterative Refinement

The benchmark results are very informative:

Key results:

| Config  |     Low      |   Working    |     High     | Iters |                       Notes                        |
|---------|--------------| ------------ | ------------ |-------|----------------------------------------------------|
| IEEE-1  | half         | float        | double       | 3     | Classic Carson & Higham baseline                   |
| IEEE-2  | bfloat16     | float        | double       | 7     | bfloat16 has fewer fraction bits → more iterations |
| IEEE-3  | float        | double       | double       | 1     | Wilkinson-style: converges in 1 iteration          |
| Posit-1 | posit<16,1>  | posit<32,2>  | posit<64,3>  | 3     | Matches IEEE-1 despite different encoding          |
| Posit-2 | posit<8,0>   | posit<16,1>  | posit<32,2>  | 8     | 8-bit factorization needs more refinement          |
| Posit-3 | posit<32,2>  | posit<64,3>  | posit<128,4> | 1     | Wide posits converge in 1 step                     |
| CF-3    | cfloat<16,4> | cfloat<32,6> | cfloat<64,8> | 2     | Extra fraction bits (fewer es) helps convergence   |
| X-2     | posit<16,1>  | float        | dd           | 2     | dd residuals accelerate convergence                |

The convergence-vs-size table shows N=100 (kappa~4053) needs 17 iterations with half/float/double — nice illustration of condition number impact.

However, many entries show 0.00e+00 for NBE/forward error because the convergence threshold is 1e-12 and the working precision can represent the exact solution well. Let me refine the convergence threshold so we see the final achieved accuracy rather than just "below 1e-12".

The issue is that once NBE/fwd reach exactly 0.0 (possible for small N=20 with double residuals), the display shows 0.00e+00. That's actually correct — the algorithm achieves exact solution to within working precision. The output is fine as-is; it's a genuine result showing that IR can recover exact solutions for small well-conditioned problems.

Let me also run with a larger N to see more interesting convergence behavior.

```bash
/home/stillwater/dev/stillwater/clones/universal/build_bench/benchmark/accuracy/blockformat/accuracy_iterative_refinement 50
  ⎿  Mixed-Precision Iterative Refinement: LU-IR with Three Precisions
     Problem: tridiag(-1, 2, -1),  N = 50,  kappa ~ 1013
     Algorithm: Carson & Higham (SIAM J. Sci. Comput., 2018)
...
```
Excellent: at N=50 (kappa~1013) we see much more interesting results:

  - IEEE-2 (bfloat16): 26 iterations and fails to converge (NBE=1.19e-8, fwd=9.54e-7). bfloat16 has only 7 fraction bits — too few for this condition number.
  - Posit-2 (posit<8,0>): Also fails to converge at 26 iterations (NBE=2.44e-5). 8-bit posit can't handle kappa~1000.
  - X-3 (bfloat16/posit<32,2>/dd): 26 iterations, NBE=1.49e-9 — the bfloat16 factorization is the bottleneck even with dd residuals.
  - Posit-1 vs IEEE-1: Both converge (3 vs 5 iters) — posit<16,1> outperforms half for this problem because it has more significand bits near 1.0.
  - CF-3 (cfloat<16,4>): Converges in 4 iters vs 5 for CF-1 (cfloat<16,5>) — the extra fraction bit (4 vs 5 es bits) helps.

The benchmark is working well and producing publication-quality data. The max_iter = 25 threshold causes those 26-iteration entries to be "failed to converge" cases, which is the correct interpretation. Let me adjust the max iter to 25 consistently and make the output show "DNF" for non-convergent cases to make this clearer, and fix the display precision to show actual values rather than 0.00e+00 when they're just very small.

Now rebuild and run with N=50 to see the "DNF" markers.

Now "DNF" (Did Not Finish) clearly marks the three non-convergent configurations:

  - IEEE-2: bfloat16 low — stalls at NBE=1.19e-8
  - Posit-2: posit<8,0> low — stalls at NBE=2.44e-5
  - X-3: bfloat16 low with dd high — stalls at NBE=1.49e-9

These are exactly the configurations where kappa(A) * u_low > 1 — the factorization is too coarse for the condition number.

The benchmark is complete and producing publication-quality output. Let me also verify the default N=20 case still works nicely.

At N=20 all configurations converge — even bfloat16 and posit<8,0> manage (7 and 8 iterations). The iteration counts tell the story:

| Low precision | Iters at N=20 | Iters at N=50 |
|---------------|---------------|---------------|
| half (fp16)   | 3             | 5             |
| bfloat16      | 7             | DNF           |
| posit<16,1>   | 3             | 3             |
| posit<8,0>    | 8             | DNF           |

The iterative refinement exposition is complete with:

  1. 5 sections: IEEE baseline, posit, cfloat, cross-family, convergence vs problem size
  2. 15 configurations spanning 4 number system families + cross-family mixing
  3. Self-contained algorithm (PLU factorization, forward/back solve, permutation)
  4. Cross-type conversion through double intermediary for mixing families
  5. DNF marking for non-convergent configurations
  6. Detailed legend with key observations and Carson & Higham reference

