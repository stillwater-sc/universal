# elreal Performance Baseline

Phase I of epic #873 (baseline) and Phase K.1 of follow-up epic #903
(small-buffer optimisation on `_components`). This document is a
baseline measurement -- not a performance target. The numbers below
come from a single workstation and are intended to identify the cost
shape of the shipped implementation, so that future optimisation work
has a starting point and a way to measure progress.

> **Phase K.1 update (#905)**: The `_components` storage migrated from
> `std::vector<double>` to a small-buffer-optimised
> `lazy_component_buffer` (inline 4 doubles + spill, see
> `include/sw/universal/number/elreal/lazy_component_buffer.hpp`).
> The headline numbers tables below carry both the original Phase I
> baseline and the post-K.1 measurements.

## Measurement setup

- **Hardware**: 12th Gen Intel(R) Core(TM) i7-12700K, single thread
- **Kernel**: Linux 6.8.0-111-generic
- **Build**: `cmake -DUNIVERSAL_BUILD_BENCHMARK_PERFORMANCE=ON` (Release, `-O3 -DNDEBUG`)
- **Compilers**: gcc 13.3.0 and clang 18.1.3
- **Benchmark source**: `benchmark/performance/arithmetic/elreal/performance.cpp`
- **Methodology**: each iteration constructs fresh elreal operands from
  double literals to avoid the captured-ancestor chain that a feedback
  pattern (`a = a + b`) would build up. Per-operation cost is reported
  via `PerformanceRunner` from `include/sw/universal/benchmark/performance_runner.hpp`.

## Headline numbers (Phase I baseline -- pre-K.1)

Throughput in operations per second, rounded. Same workload, two compilers.
Both elreal and ereal<N> workloads construct fresh operands inside the
loop body so the per-iteration allocation pattern matches between sides.
These were the numbers before the K.1 small-buffer optimisation:

| Operation | Budget | gcc 13.3 | clang 18.1 |
|---|---|---:|---:|
| `elreal +` | depth 0 | 9 Mops/s | 7 Mops/s |
| `elreal +` | depth 1 | 9 Mops/s | 6 Mops/s |
| `elreal -` | depth 1 | 9 Mops/s | 7 Mops/s |
| `elreal *` | depth 1 | 8 Mops/s | 4 Mops/s |
| `elreal /` | depth 0 | 36 Mops/s | 23 Mops/s |
| `elreal sqrt` | depth 1 | 14 Mops/s | 13 Mops/s |
| `elreal exp` | depth 1 | 14 Mops/s | 13 Mops/s |
| `elreal log` | depth 1 | 14 Mops/s | 13 Mops/s |
| `elreal + refine_to(106)` | --- | 9 Mops/s | 9 Mops/s |
| `elreal + refine_to(212)` | --- | 8 Mops/s | 8 Mops/s |
| `ereal<2> +` | --- | 24 Mops/s | 25 Mops/s |
| `ereal<2> -` | --- | 19 Mops/s | 26 Mops/s |
| `ereal<2> *` | --- | 10 Mops/s | 10 Mops/s |
| `ereal<2> /` | --- | 650 Kops/s | 727 Kops/s |
| `ereal<4> /` | --- | 639 Kops/s | 721 Kops/s |
| `ereal<8> /` | --- | 636 Kops/s | 725 Kops/s |

## Headline numbers (post-K.1, current)

After the K.1 small-buffer optimisation. Same workload, same hardware,
same compilers. ereal numbers are unchanged from above (K.1 only touched
elreal):

| Operation | Budget | gcc 13.3 | clang 18.1 | vs Phase I (gcc) |
|---|---|---:|---:|---:|
| `elreal +` | depth 0 | 16 Mops/s | 17 Mops/s | **1.8x faster** |
| `elreal +` | depth 1 | 12 Mops/s | 21 Mops/s | **1.3x faster** |
| `elreal -` | depth 1 | 14 Mops/s | 17 Mops/s | **1.6x faster** |
| `elreal *` | depth 1 | 19 Mops/s | 22 Mops/s | **2.4x faster** |
| `elreal /` | depth 0 | 1 Gops/s | 138 Mops/s | dominated by compiler inlining once heap alloc is gone (see note) |
| `elreal sqrt` | depth 1 | 30 Mops/s | 30 Mops/s | **2.1x faster** |
| `elreal exp` | depth 1 | 31 Mops/s | 34 Mops/s | **2.2x faster** |
| `elreal log` | depth 1 | 24 Mops/s | 28 Mops/s | **1.7x faster** |
| `elreal + refine_to(106)` | --- | 13 Mops/s | 15 Mops/s | 1.4x |
| `elreal + refine_to(212)` | --- | 11 Mops/s | 17 Mops/s | 1.4x |

The two compilers no longer differ materially on most operators -- both
land in the same 12-22 Mops/s range for arithmetic. The clang gap on
`elreal *` that the Phase I baseline flagged (4 vs 8 Mops/s) is closed.

The `elreal /` Gops/s result is genuine in the workload but worth
flagging: with the inline-buffer change, the result `elreal` is fully
stack-allocatable, and `elreal::operator/` happens to be the simplest
operator (single double divide, no captured generator -- depth-2+
Newton refinement is deferred to Phase L #906). gcc inlines the whole
operator and the only remaining work is the double divide itself. In a
workload where the result needs to be propagated into a more complex
expression, the throughput drops back to the same range as the other
operators.

Below, we use the gcc 13.3 post-K.1 numbers as the reference unless
otherwise noted.

## Reading the table

### elreal arithmetic at depth 1

`+`, `-`, `*` all land in the 4-9 Mops/s range. The cost shape is
dominated by:

1. **One `std::vector<double>` allocation** for `_components` per result.
2. **One `std::function` capture** for the generator (heap-allocated by
   libstdc++ / libc++ since the capture exceeds the small-buffer
   optimisation threshold -- two `elreal` copies plus a `sum_err` double).
3. **Two `elreal` copy constructions** to bind into the lambda capture.
   Each copy walks the source's `_components` vector and increments any
   reference counts on its `std::function`.

The two_sum / two_prod EFTs themselves are inexpensive -- they are a few
double FLOPs each. The per-op cost is overwhelmingly memory traffic
(allocation, vector population, function-object packing).

### elreal `/` at depth 0

Division clocks 23-36 Mops/s -- three to six times faster than the other
arithmetic operators. The reason: as of Phase G, `elreal::operator/`
materialises `c0 = a.at(0) / b.at(0)` (a single double division) and
returns a degenerate generator (Newton refinement is deferred to
future work). So there is no captured-ancestor lambda and no second
component to compute. Once Newton iteration is in place, division will
look much more like `*`.

### elreal math (sqrt / exp / log)

13-14 Mops/s -- *faster* than `+`/`-`/`*` despite computing an `std::sqrt`
or `std::exp` inside. The reason is structural: the math operators
capture one operand into the lambda instead of two. Allocation is the
bottleneck, and saving one `elreal` copy in the capture is worth more
than the cost of a `std::sqrt` call on a modern x86 core.

The clang-vs-gcc gap on `elreal *` (4 vs 8 Mops/s) is worth flagging:
multiplication is the operator with the heaviest captured payload
(both inputs, plus the `two_prod` error term), and libc++'s default
allocator hits the small-object pool harder than libstdc++'s glibc
malloc here. Both compilers land in the same shape; the absolute gap
is the kind of thing the Phase II small-buffer optimisation on
`_components` would close.

### elreal `refine_to(106)` vs `refine_to(212)`

These are both ~ 8-9 Mops/s, only slightly slower than depth 1. This is
the *current* shape of the implementation: each operator's generator
clamps at depth 1 (returns 0 for k >= 2). So requesting 212 bits via
`refine_to` walks the generator once for k=1 and then writes zeros for
k=2 through ceil(212/53). Once depth-2+ refinement lands (Newton for `/`
and `sqrt`, higher-precision internal algorithms for the rest), this
sweep will become informative.

### ereal<N> at matched precision

`ereal<2>` (~ 106 bits, dd-equivalent) clocks 10-26 Mops/s for `+ - *`
when its operands are also freshly allocated each iteration (matched
to the elreal workload pattern) -- roughly **1.2-3x faster than elreal**
at the same precision target. The gap holds across `ereal<4>` and
`ereal<8>`: ereal's component vector is a `std::vector<double>` like
elreal's, but ereal carries no captured-generator lambda alongside it,
so the allocation cost per op is roughly half. The narrower gap on
multiplication (1.2x rather than 3x) reflects that ereal's
`expansion_product` does O(N) work per op, eroding its allocator
advantage as N grows.

The exception is `ereal<N> /`, which is **~50x slower than elreal /**
at depth 0 (650 Kops/s vs 36 Mops/s for ereal<2> vs elreal). ereal's
division runs iteratively until it has enough components; elreal's
current division just does a single double divide and returns. The
right reading: elreal's division is currently more *throughput-friendly*
but at lower precision -- a fair comparison requires Newton refinement
in elreal first.

## When is `elreal` faster than `ereal`?

The picture changed materially with K.1:

| Op | `elreal` post-K.1 (gcc) | `ereal<2>` (gcc) | Winner |
|---|---:|---:|---|
| `+` | 12 Mops/s | 24 Mops/s | `ereal<2>` (~ 2x) |
| `-` | 14 Mops/s | 19 Mops/s | `ereal<2>` (~ 1.4x) |
| `*` | 19 Mops/s | 10 Mops/s | **`elreal` (~ 1.9x)** |
| `/` | (apples-to-oranges) | 650 Kops/s | -- |
| `sqrt`, `exp`, `log` | 24-31 Mops/s | n/a | `elreal` only |

Multiplication has flipped: `elreal *` now beats `ereal<2> *` at matched
precision because `ereal<N>` multiplication is O(N) in the eager
expansion product while `elreal *` is essentially a single `two_prod`
plus the (now inline) result envelope.

Addition and subtraction still favour `ereal<2>` -- those operators
are O(1) in `ereal<N>` for small N and the per-iteration cost is
dominated by the result construction, which `ereal<N>` already amortises
better than the lazy-stream envelope. The remaining gap is what
Phase K.2 (`std::function` -> tagged-union generator) and Phase K.3
(reference-counted operand sharing) target.

What `elreal` gives you instead, and what `ereal` cannot:

1. **Decidable sign.** Comparison walks the stream until a non-zero
   difference is found, with a bounded budget. `ereal` has no equivalent;
   equality on `ereal<N>` is bitwise on the array, which is *not*
   mathematical equality.
2. **Per-call precision growth.** `elreal::refine_to(B)` lets one call
   in a thousand pay for deeper precision without changing the
   committed budget on the type. `ereal<N>` is precision-fixed at the
   instantiation site.
3. **General-position skip.** For workloads where the *common case*
   needs depth 0 (most geometric predicates on shuffled inputs), elreal's
   depth-0 cost is the single double op plus the lazy-result envelope
   -- the envelope is what we are paying for above, and shrinking it is
   the natural Phase II work.

**Conclusion**: `elreal` wins on *correctness* in cases where `ereal`
cannot answer, not on throughput at matched precision today. The picker
decision tree in `multi-component-arithmetic.md` reflects this.

## Allocation hot path -- candidates for tuning

Profile-guided observation: the bottleneck on every arithmetic operator
is the same triple of `_components` vector allocation, `_generator`
function-object packing, and the input copies that go into that pack.

Concrete Phase K candidates of the follow-up epic (#905), in order of
expected payoff:

1. ~~**Small-buffer optimisation on `_components`**~~ -- **DONE in K.1**.
   `std::vector<double>` replaced with `lazy_component_buffer` (inline 4
   doubles + spill via `std::vector`). The common case (depth 1-4)
   pays no heap allocation. Achieved 1.3-2.4x speedup across
   arithmetic and math at matched precision.
2. **Generator type erasure that avoids heap** (Phase K.2). `std::function`
   always heap-allocates when the capture exceeds the SBO. Replacing
   `std::function<double(std::size_t)>` with a tagged-union or
   intrusive-list-of-known-shapes representation would eliminate the
   second per-op allocation. The known shapes are small: depth-1 EFT
   residuals, depth-1 derivative corrections, constants, degenerate
   (all-zero). Each is a fixed-size POD.
3. **Reference-counted operand sharing** (Phase K.3). The lambda captures
   *copies* of both inputs. Switching to `std::shared_ptr<const Components>`
   would let multiple results share an ancestor without copying the
   component vector. This becomes more valuable once Phase L's depth-2+
   generators chain back to an ancestor.
4. **SIMD/FMA on `two_sum` and `two_prod` batches** (Phase K.4). Once
   the allocation cost is shrunk, the EFT primitives become a
   non-trivial fraction of the loop. A batch interface that processes
   4-8 EFTs in one SIMD pass would help reductions and dot products.

K.2 is the natural next target now that K.1 has shrunk the
component-vector allocation: the `std::function` capture is the
remaining per-op allocation cost.

## Out of scope for Phase I

- **MPFR comparison.** MPFR would be the right oracle benchmark for an
  independent precision target. Adding it requires an opt-in
  third-party dependency and a separate CMake path. Deferred per the
  Phase I issue scope.
- **BLAS-scale runs.** Single-operation throughput is what we care about
  for the picker decision tree. Vector/matrix performance lives in
  `benchmark/performance/blas/` and will be a Phase II item once the
  allocator hot path is addressed (today the answer would be "elreal is
  too slow to use in a BLAS L3 kernel").
- **Chain-length scaling study.** The `a = a + b` feedback pattern grows
  the captured-ancestor chain linearly with iteration count, and the
  `at(1)` walk through that chain dominates. The baseline benchmark
  uses fresh operands to factor this out, but the scaling itself is
  worth a dedicated experiment -- earmarked for Phase II.

## Reproducing these numbers

```bash
mkdir build && cd build
cmake -DUNIVERSAL_BUILD_BENCHMARK_PERFORMANCE=ON ..
make benchmark_elreal_performance -j4
./benchmark/performance/arithmetic/benchmark_elreal_performance
```

Per-machine variance is typically within 10-20% for these op counts on
the same hardware between runs. For tracking optimisation progress,
running 3-5 times and taking the median is the recommended protocol.

## References

- `benchmark/performance/arithmetic/elreal/performance.cpp` -- benchmark source
- `include/sw/universal/benchmark/performance_runner.hpp` -- timing primitive
- `docs/number-systems/elreal.md` -- user-facing API
- `docs/algorithmic-details/lazy-real-arithmetic.md` -- algorithmic
  deep-dive (refinement budget, depth-1 derivative correction, why
  forward trig pays the std-library price at depth 0)
- `docs/algorithmic-details/multi-component-arithmetic.md` -- picker
  decision tree with the elreal-vs-ereal boundary
