# elreal Performance Baseline

Phase I of epic #873 (baseline), Phase K.1 (#912), and Phase K.2 (#905)
of follow-up epic #903. This document is a baseline measurement -- not
a performance target. The numbers below come from a single workstation
and are intended to identify the cost shape of the shipped
implementation, so that future optimisation work has a starting point
and a way to measure progress.

> **Phase K.1 update**: The `_components` storage migrated from
> `std::vector<double>` to a small-buffer-optimised
> `lazy_component_buffer` (inline 4 doubles + spill).
>
> **Phase K.2 update**: `_generator` migrated from
> `std::function<double(std::size_t)>` to a `std::variant` of small POD
> shapes (`gen_unary_linear`, `gen_binary_linear`, `gen_sqrt`,
> `gen_unary_neg`, `gen_rational_residual`). Operand captures are now
> `std::shared_ptr<const elreal>` (16 bytes) rather than `elreal` by
> value (~104 bytes), eliminating the per-op std::function heap
> allocation that the original Phase I baseline identified as the
> dominant per-op cost.

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

## Headline numbers (post-K.1)

After the K.1 small-buffer optimisation. Numbers retained for the
before/after comparison; current numbers are the post-K.2 table further
down.

| Operation | Budget | gcc 13.3 | clang 18.1 |
|---|---|---:|---:|
| `elreal +` | depth 0 | 16 Mops/s | 17 Mops/s |
| `elreal +` | depth 1 | 12 Mops/s | 21 Mops/s |
| `elreal -` | depth 1 | 14 Mops/s | 17 Mops/s |
| `elreal *` | depth 1 | 19 Mops/s | 22 Mops/s |
| `elreal sqrt` | depth 1 | 30 Mops/s | 30 Mops/s |
| `elreal exp` | depth 1 | 31 Mops/s | 34 Mops/s |
| `elreal log` | depth 1 | 24 Mops/s | 28 Mops/s |

## Headline numbers (post-K.2, current)

After the K.2 variant-generator + shared-operand changes. The math
functions show the largest gains because the unary derivative captures
(1 shared_ptr + 1 double) fit naturally into the new shape:

| Operation | Budget | gcc 13.3 | clang 18.1 | vs Phase I (gcc) |
|---|---|---:|---:|---:|
| `elreal +` | depth 0 | 20 Mops/s | 16 Mops/s | **2.2x faster** |
| `elreal +` | depth 1 | 17 Mops/s | 11 Mops/s | **1.9x faster** |
| `elreal -` | depth 1 | 17 Mops/s | 14 Mops/s | **1.9x faster** |
| `elreal *` | depth 1 | 16 Mops/s | 15 Mops/s | **2.0x faster** |
| `elreal /` | depth 0 | 1 Gops/s | 64 Mops/s | dominated by inlining once heap alloc is gone |
| `elreal sqrt` | depth 1 | 36 Mops/s | 18 Mops/s | **2.6x faster** |
| `elreal exp` | depth 1 | 43 Mops/s | 26 Mops/s | **3.1x faster** |
| `elreal log` | depth 1 | 38 Mops/s | 31 Mops/s | **2.7x faster** |
| `elreal + refine_to(106)` | --- | 20 Mops/s | 19 Mops/s | 2.2x |
| `elreal + refine_to(212)` | --- | 14 Mops/s | 19 Mops/s | 1.8x |

The K.1 baseline already eliminated the `_components` vector alloc;
K.2 eliminates the `_generator` std::function alloc. Together they
take per-binary-op heap allocations from 2 (vector + function) to
roughly 2 small allocs (one make_shared per operand) -- but each
allocation is now small and fixed-size, friendly to the allocator's
fast path.

Below, we use the gcc 13.3 post-K.2 numbers as the reference unless
otherwise noted.

## Reading the table (current, post-K.2)

### elreal arithmetic at depth 1

`+`, `-`, `*` now land in the 16-17 Mops/s range -- a roughly 2x
improvement on the original Phase I baseline (8-9 Mops/s). The
remaining cost is:

1. **Two `std::make_shared<const elreal>(...)` allocations** per binary
   op for the operand handles. Each is a small fixed-size alloc
   (sizeof control block + sizeof elreal) that hits the allocator's
   fast path.
2. **The variant emplace** (no allocation; the variant alternatives are
   stored inline in the result `elreal`).
3. **The leading-component EFT computation** (`two_sum`/`two_diff`/`two_prod`).

The K.1 vector alloc and K.2 std::function alloc are both gone. The
two_sum / two_prod EFTs are a small number of double FLOPs each. The
per-op cost is now roughly balanced between the small allocations and
the leading-component math.

### elreal `/` at depth 1 (Phase L.1, #916-or-later)

After Phase L.1, division has depth-1 refinement matching the other
arithmetic operators: a `gen_binary_linear` generator with the
constant term carrying the IEEE-residual (`a - b*c0`, computed
exactly via `two_prod` + `two_diff`) divided by `b0`, and operand
corrections via the Taylor partials `1/b0` and `-c0/b0`. Throughput
lands in the 13-16 Mops/s range -- the same as `+`/`-`/`*`.

The pre-L.1 baseline had division clocking ~ 1 Gops/s on gcc, which
was an artifact of the compiler inlining through the entire depth-0
operator (single double divide, no captured generator). That benchmark
shape no longer holds; the post-L.1 number reflects real depth-1
arithmetic work.

Depth 2+ via Newton iteration is Phase L.2 follow-up work.

### elreal math (sqrt / exp / log)

36-43 Mops/s on gcc -- *faster* than `+`/`-`/`*` despite computing a
`std::sqrt` or `std::exp` internally. The reason is structural: the
unary math operators capture one operand into a `gen_unary_linear`
(1 shared_ptr + 1 double = 24 bytes inline), so the per-op envelope
is the smallest among the operators. Allocation is the bottleneck,
and saving one shared_ptr alloc in the capture is worth more than
the cost of a `std::sqrt` call on a modern x86 core.

### elreal `refine_to(106)` vs `refine_to(212)`

Both ~ 14-20 Mops/s, only slightly slower than depth 1. Each
operator's generator clamps at depth 1 (returns 0 for k >= 2), so
requesting 212 bits via `refine_to` walks the generator once for
k=1 and then writes zeros for k=2 through ceil(212/53). Once
depth-2+ refinement lands (Phase L for Newton on `/` and `sqrt`,
Phase M for higher-precision algorithms on the transcendentals),
this sweep will become informative.

### ereal<N> at matched precision

`ereal<2>` (~ 106 bits, dd-equivalent) clocks 19-27 Mops/s for
`+ - *` when its operands are also freshly allocated each iteration
(matched to the elreal workload pattern). With K.2 in place, the
arithmetic gap with `elreal` has narrowed substantially:

| Op | `elreal` post-K.2 (gcc) | `ereal<2>` (gcc) | Winner |
|---|---:|---:|---|
| `+` | 17 Mops/s | 19 Mops/s | `ereal<2>` (~ 1.1x; gap nearly closed) |
| `-` | 17 Mops/s | 20 Mops/s | `ereal<2>` (~ 1.2x) |
| `*` | 16 Mops/s | 11 Mops/s | **`elreal` (~ 1.5x)** |
| `/` (depth 1 post-L.1) | 13 Mops/s | 680 Kops/s | **`elreal` (~ 19x)** |
| `sqrt`, `exp`, `log` | 36-43 Mops/s | n/a | `elreal` only |

Multiplication continues to favour `elreal`: `ereal<N>` multiplication
is O(N) in the eager expansion product while `elreal *` is essentially
a single `two_prod` plus the (now inline) result envelope.

Division also favours `elreal` after Phase L.1 (#906). `ereal<N> /`
runs the iterative `expansion_quotient` algorithm (~ 680 Kops/s);
`elreal /` produces a depth-1 result via a single `gen_binary_linear`
generator with the IEEE residual + Taylor partials baked into the
coefficients. Both deliver the same ~ 106-bit precision target at
depth 1.

## When is `elreal` faster than `ereal`?

After K.2, `elreal` is competitive with `ereal<2>` on every elementary
arithmetic operator at matched precision, and *wins* on multiplication
and on every math function (the entire math suite is `elreal`-only
since `ereal<N>` does not expose math functions today).

What `elreal` provides that `ereal<N>` cannot, independent of throughput:

1. **Decidable sign.** Comparison walks the stream until a non-zero
   difference is found, with a bounded budget. `ereal` has no equivalent;
   equality on `ereal<N>` is bitwise on the array, which is *not*
   mathematical equality.
2. **Per-call precision growth.** `elreal::refine_to(B)` lets one call
   in a thousand pay for deeper precision without changing the
   committed budget on the type. `ereal<N>` is precision-fixed at the
   instantiation site.
3. **General-position skip.** For workloads where the *common case*
   needs depth 0 (most geometric predicates on shuffled inputs),
   elreal's depth-0 cost is the single double op plus the lazy-result
   envelope.

**Conclusion**: `elreal` is now both *throughput-competitive* at matched
precision *and* offers correctness features `ereal<N>` lacks. The
picker decision tree in `multi-component-arithmetic.md` reflects this.

## Historical: Phase I baseline analysis (pre-K.1)

Retained for archival reference. The numbers and the cost-shape
discussion below describe the pre-optimisation state.

The Phase I baseline (#902) measured the elreal arithmetic at
4-9 Mops/s for `+`/`-`/`*` on gcc 13.3, and identified the dominant
cost as:

1. **One `std::vector<double>` allocation** for `_components` per
   result. (Closed by K.1.)
2. **One `std::function` capture** for the generator (heap-allocated
   by libstdc++/libc++ since the capture exceeded the SBO threshold
   -- two `elreal` copies plus a residual double). (Closed by K.2.)
3. **Two `elreal` copy constructions** into the lambda capture.
   (Closed by K.2; operands now shared via `shared_ptr<const elreal>`.)

The math functions (sqrt, exp, log) at the Phase I baseline clocked
13-14 Mops/s -- already faster than arithmetic because the unary
captures fit in a smaller `std::function` payload. K.2 widened that
advantage.

The pre-K.2 picker rule said `ereal<2>` won on `+/-/*` by 1.2-3x at
matched precision; only multiplication favoured `elreal` after K.1
(1.9x). After K.2, the gap on `+/-` is within ~ 20% and multiplication
still favours `elreal` by 1.5x.

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
