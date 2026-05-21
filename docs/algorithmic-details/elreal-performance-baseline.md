# elreal Performance Baseline

Phase I of epic #873. This document is a baseline measurement -- not a
performance target. The numbers below come from a single workstation and
are intended to identify the cost shape of the shipped implementation, so
that future optimisation work has a starting point and a way to measure
progress.

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

## Headline numbers

Throughput in operations per second, rounded. Same workload, two compilers:

| Operation | Budget | gcc 13.3 | clang 18.1 |
|---|---|---:|---:|
| `elreal +` | depth 0 | 6 Mops/s | 7 Mops/s |
| `elreal +` | depth 1 | 5 Mops/s | 5 Mops/s |
| `elreal -` | depth 1 | 5 Mops/s | 7 Mops/s |
| `elreal *` | depth 1 | 7 Mops/s | 9 Mops/s |
| `elreal /` | depth 0 | 35 Mops/s | 35 Mops/s |
| `elreal sqrt` | depth 1 | 13 Mops/s | 14 Mops/s |
| `elreal exp` | depth 1 | 13 Mops/s | 14 Mops/s |
| `elreal log` | depth 1 | 13 Mops/s | 14 Mops/s |
| `elreal + refine_to(106)` | --- | 9 Mops/s | 9 Mops/s |
| `elreal + refine_to(212)` | --- | 8 Mops/s | 8 Mops/s |
| `ereal<2> +` | --- | 35 Mops/s | 39 Mops/s |
| `ereal<2> *` | --- | 12 Mops/s | 13 Mops/s |
| `ereal<2> /` | --- | 677 Kops/s | 753 Kops/s |
| `ereal<4> /` | --- | 640 Kops/s | 743 Kops/s |
| `ereal<8> /` | --- | 660 Kops/s | 734 Kops/s |

The two compilers track within a few percent across the board. Below, we
use the gcc 13.3 numbers as the reference unless otherwise noted.

## Reading the table

### elreal arithmetic at depth 1

`+`, `-`, `*` all land in the 5-9 Mops/s range. The cost shape is
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

Division clocks ~35 Mops/s -- five to seven times faster than the other
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

### elreal `refine_to(106)` vs `refine_to(212)`

These are both ~ 8-9 Mops/s, only slightly slower than depth 1. This is
the *current* shape of the implementation: each operator's generator
clamps at depth 1 (returns 0 for k >= 2). So requesting 212 bits via
`refine_to` walks the generator once for k=1 and then writes zeros for
k=2 through ceil(212/53). Once depth-2+ refinement lands (Newton for `/`
and `sqrt`, higher-precision internal algorithms for the rest), this
sweep will become informative.

### ereal<N> at matched precision

`ereal<2>` (~ 106 bits, dd-equivalent) clocks 12-35 Mops/s for `+ - *` --
roughly **3-7x faster than elreal** at the same precision target. The
gap holds across `ereal<4>` and `ereal<8>`, which is the expected
shape: ereal's storage is a fixed-size `std::array<double, N>`, no heap
allocation per op, and the eager expansion runs out of cache lines
quickly enough to stay in L1.

The exception is `ereal<N> /`, which is **~50x slower than elreal /**
at depth 0 (677 Kops/s vs 35 Mops/s for ereal<2> vs elreal). ereal's
division runs iteratively until it has enough components; elreal's
current division just does a single double divide and returns. The
right reading: elreal's division is currently more *throughput-friendly*
but at lower precision -- a fair comparison requires Newton refinement
in elreal first.

## When is `elreal` faster than `ereal`?

At today's depth-1 cap, `elreal` is essentially never faster than `ereal`
on the elementary arithmetic when measured in raw ops per second.
`ereal<2>` wins at all four operations except division, where the lazy
shortcut produces a misleading apples-to-oranges win.

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

Concrete Phase II candidates, in order of expected payoff:

1. **Small-buffer optimisation on `_components`.** A `std::vector<double>`
   of size 1-2 is the common case. A `small_vector<double, 4>` (or a
   `std::array<double, K>` with a runtime `size_`) would eliminate one
   allocation per op for the typical case. This is the single largest
   item on the list.
2. **Generator type erasure that avoids heap.** `std::function` always
   heap-allocates when the capture exceeds the SBO. Replacing
   `std::function<double(std::size_t)>` with a tagged-union or
   intrusive-list-of-known-shapes representation would eliminate the
   second per-op allocation. The known shapes are small: depth-1 EFT
   residuals, depth-1 derivative corrections, constants, degenerate
   (all-zero). Each is a fixed-size POD.
3. **Reference-counted operand sharing.** The lambda captures *copies*
   of both inputs. Switching to `std::shared_ptr<const Components>`
   would let multiple results share an ancestor without copying the
   component vector. This becomes more valuable once Phase II depth-2+
   generators chain back to an ancestor.
4. **SIMD/FMA on `two_sum` and `two_prod` batches.** Once the
   allocation cost is shrunk, the EFT primitives become a non-trivial
   fraction of the loop. A batch interface that processes 4-8 EFTs in
   one SIMD pass would help reductions and dot products. This is a
   later step -- meaningful only after the allocator hot path is
   addressed.

None of these are committed to a specific Phase II PR by this baseline;
they are the natural ordering for follow-up work.

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
