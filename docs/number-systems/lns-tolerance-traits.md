# LNS Algorithm Tolerance Traits

The configurable [add/sub algorithms](../lns-addsub-algorithms/)
deliberately sacrifice bit-exact accuracy for speed, SRAM, or energy.
That makes a regression test that compares `c = a + b` (in LNS) against
`encode(double(a) + double(b))` (the oracle) too strict to use across
all algorithms -- the approximate ones will fail even when they are
within their documented error envelopes.

This page covers Universal's solution: a per-algorithm log-domain error
bound trait and a `rbits`-aware comparison helper that auto-derives the
correct value-domain tolerance for any `(algorithm, lns<>)` pair.

## The problem: boundary-rounding amplification

Concrete example. With `lns<8, 2>` and `ArnoldBaileyAddSub`:

- `lns<8, 2>` precision: `rbits = 2`, so the log-domain ULP is
  `2^-2 = 0.25` and adjacent encoded values differ by a factor of
  `2^0.25 ~= 1.189` -- about 19% in value-domain relative spacing.
- `ArnoldBailey` worst-case `sb_add` error: ~0.025 in the log domain
  (the secant-interpolation error mid-interval, near `d = -0.5`).

(Recall: `sb_add(d) = log2(1 + 2^d)` is the log-add correction defined
in [Introduction](../lns/#how); the `sb_` prefix is documented there.)

The algorithm error 0.025 is much smaller than the ULP 0.25, so the
algorithm is accurate "in absolute terms." But: when the true result
sits near a rounding boundary, the algorithm's small log-domain error
can push the encoded result *across the boundary* into the adjacent
ULP. One ULP shift in `lns<8, 2>` is a 19% relative jump in the value
domain.

So a bit-exact regression test will see ArnoldBailey produce values
that are encoded to a different ULP than the oracle -- even though the
algorithm's log-domain accuracy is well within its documented envelope.
About 10-20% of operand pairs near the secant midpoint exhibit this
boundary flip.

The same arithmetic at `lns<16, 8>` looks completely different:
`rbits = 8`, log-domain ULP is `2^-8 ~= 0.004`, value-domain ULP is
`~0.27%`, and the algorithm error of 0.025 in log domain shifts the
encoded result by `0.025 / 0.004 ~= 6` ULPs -- still about ~2% in the
value domain (because each ULP is so small).

A flat per-algorithm value-domain tolerance is wrong by orders of
magnitude across configurations. The tolerance has to scale with
`rbits`.

## The trait: `lns_addsub_log_error_bound`

Each shipped algorithm advertises an upper bound on its `sb_add` /
`sb_sub` error, **in the log domain** (where the bound is independent
of the LNS resolution):

```cpp
template<typename Alg>
struct lns_addsub_log_error_bound {
    static constexpr double value = 0.0;  // default: exact
};

// Per-policy specializations:
template<typename Lns>
struct lns_addsub_log_error_bound<DoubleTripAddSub<Lns>> {
    static constexpr double value = 0.0;
};
template<typename Lns>
struct lns_addsub_log_error_bound<DirectEvaluationAddSub<Lns>> {
    static constexpr double value = 0.0;
};
template<typename Lns, unsigned IndexBits>
struct lns_addsub_log_error_bound<LookupAddSub<Lns, IndexBits>> {
    static constexpr double value = 1.0e-4;  // default IndexBits
};
template<typename Lns>
struct lns_addsub_log_error_bound<PolynomialAddSub<Lns>> {
    static constexpr double value = 1.0e-5;  // degree-7 truncation
};
template<typename Lns>
struct lns_addsub_log_error_bound<ArnoldBaileyAddSub<Lns>> {
    static constexpr double value = 2.5e-2;  // piecewise-linear secant
};
```

A value of `0.0` means *exact* -- the algorithm matches the oracle
bit-for-bit (`DoubleTrip` and `Direct`). Any positive value is the
per-operation log-domain error budget.

These bounds match the envelopes measured by the benchmark in
`benchmark/performance/arithmetic/lns/log_add_algorithms.cpp` and have
small safety margins built in.

## The helper: `lns_eq_within_alg_tolerance`

```cpp
template<typename LnsType>
constexpr bool lns_eq_within_alg_tolerance(const LnsType& c, const LnsType& cref);
```

Compares two `lns<>` values using the active algorithm's tolerance
contract:

- Looks up `lns_addsub_algorithm_t<LnsType>` (the algorithm in use)
- Looks up `lns_addsub_log_error_bound_v<Alg>` (its log-domain bound)
- For exact algorithms (`E == 0.0`), does bit-exact `c == cref`
- For approximate algorithms, derives a value-domain relative tolerance
  from `(E, LnsType::rbits)` and compares within that bound

NaN handling is uniform: both-NaN counts as equivalent, one-sided NaN
is a fail. This holds across exact and approximate algorithms.

### How the rbits-aware tolerance is derived

```text
log_ulp   = 2^-rbits                              // one LNS ULP in log domain
ulp_shift = (E / log_ulp) + 2                     // worst-case ULP shift, +2 noise margin
rel_tol   = 2^(ulp_shift * log_ulp) - 1           // value-domain relative tolerance
```

The first line is just the LNS encoding granularity. The second line
says: the algorithm's log-domain error `E` can shift the encoded result
by at most `ceil(E / log_ulp)` ULPs, plus 2 ULPs of margin to absorb
half-ULP rounding on each side of a boundary. The third line converts
that ULP-shift back to a value-domain relative tolerance: a shift of
`N` ULPs corresponds to a multiplicative factor of `2^(N * log_ulp)`,
so the relative error is `2^(N * log_ulp) - 1`.

How this scales:

| `(Alg, rbits)`          | E      | log_ulp | ULP shift | `rel_tol`               |
|-------------------------|--------|---------|-----------|-------------------------|
| `Direct, *`             | 0      | -       | -         | bit-exact (`c == cref`) |
| `Polynomial, 8`         | 1e-5   | 0.0039  | ~3        | ~0.8%                   |
| `Polynomial, 16`        | 1e-5   | 1.5e-5  | ~3        | bound saturates to ~3e-5|
| `Lookup, 8`             | 1e-4   | 0.0039  | ~4        | ~1.1%                   |
| `Lookup, 16`            | 1e-4   | 1.5e-5  | ~9        | bound saturates to ~1e-4|
| `ArnoldBailey, 2`       | 2.5e-2 | 0.25    | 2         | **~41%** (allows boundary flip) |
| `ArnoldBailey, 8`       | 2.5e-2 | 0.0039  | ~8        | ~2.2%                   |
| `ArnoldBailey, 16`      | 2.5e-2 | 1.5e-5  | ~1638     | ~2.4% (algo bound dominates) |

Note the `ArnoldBailey, rbits=2` row: at low precision a single ULP
shift is already 19% in the value domain, and the algorithm's
log-domain error (0.025) is enough to occasionally push results 1 ULP
across a boundary. The 41% tolerance is what allows that 1-ULP flip
without flagging it as a regression. At higher precision the
algorithm's intrinsic accuracy bound takes over.

## Wiring it into the regression suite

The regression verifiers in
`include/sw/universal/verification/lns_test_suite.hpp` use the helper
in place of bit-exact comparison:

```cpp
template<typename LnsType, std::enable_if_t<is_lns<LnsType>, bool> = true>
int VerifyAddition(bool reportTestCases) {
    // ... exhaustive loop over (a, b) operand pairs ...
    c    = a + b;
    cref = ref;
    if (!lns_eq_within_alg_tolerance(c, cref)) {
        ++nrOfFailedTestCases;
        if (reportTestCases)
            ReportBinaryArithmeticError("FAIL", "+", a, b, c, cref);
    } else {
        if (reportTestCases)
            ReportBinaryArithmeticSuccess("PASS", "+", a, b, c, ref);
    }
    // ...
}
```

For the default `DoubleTripAddSub` the helper short-circuits to
`c == cref`, so existing regression tests see no behavior change. For
specialized algorithms the helper auto-derives the right tolerance
from the trait + the `LnsType::rbits`.

## Worked example: `lns<8, 2>` + `ArnoldBailey`

This is the case that motivated the trait. With the default trait
(DoubleTrip), the bit-exact comparison passes. With ArnoldBailey
specialized in:

```cpp
namespace sw::universal {
    template<>
    struct lns_addsub_traits<lns<8, 2, std::uint8_t>> {
        using type = ArnoldBaileyAddSub<lns<8, 2, std::uint8_t>>;
    };
}

int failed = VerifyAddition<lns<8, 2, std::uint8_t>>(false);
// failed == 0 -- passes within ArnoldBailey's documented envelope
```

What is happening under the hood: the helper computes
`rel_tol ~= 2^((0.025 / 0.25 + 2) * 0.25) - 1 ~= 0.41` (about 41% in
the value domain). That generously covers the 19% ULP shift that
ArnoldBailey's log-domain error can produce on `lns<8, 2>`, while
still flagging actual regressions (anything that pushes the result
beyond a single-ULP boundary flip on this configuration).

## Contract for users specializing custom algorithms

If you ship a custom `sb_add` / `sb_sub` policy, you must specialize
**both** traits:

```cpp
namespace sw::universal {
    template<typename Lns>
    struct MyAddSub { /* sb_add, sb_sub, add_assign, sub_assign */ };

    // Tolerance trait: bound on |sb_add - true_sb_add| in log domain
    template<typename Lns>
    struct lns_addsub_log_error_bound<MyAddSub<Lns>> {
        static constexpr double value = 1.0e-3;  // your measured bound
    };

    // Algorithm trait: opt your lns<> instance into MyAddSub
    template<>
    struct lns_addsub_traits<lns<32, 16, std::uint32_t>> {
        using type = MyAddSub<lns<32, 16, std::uint32_t>>;
    };
}
```

If you forget the tolerance trait specialization, the default
(`value = 0.0`) is used and the regression suite will treat your
algorithm as exact -- which means any approximation will be flagged as
a failure. This is the right default: silently accepting unknown error
bounds would mask real bugs.

## See also

- [Introduction](../lns/) for the LNS history and Gauss log-add math
- [Implementation](../lns-implementation/) for `lns<>` template details
- [Add/Sub Algorithms](../lns-addsub-algorithms/) for the five shipped
  policies and their accuracy bounds
- The header
  `include/sw/universal/number/lns/lns_addsub_algorithms.hpp` for the
  trait + helper definitions
- The shared verifier at
  `include/sw/universal/verification/lns_test_suite.hpp` for the
  regression integration point
