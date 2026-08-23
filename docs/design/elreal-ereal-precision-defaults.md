# Choosing a precision knob for `elreal` and `ereal`

Measured guidance for the two adaptive-precision real types, from the
characterization tool in `benchmark/accuracy/adaptive/`. The raw sweep this page is
built from is committed beside the tool as `baseline-characterization.txt`.

Regenerate with:

```bash
characterize 8 5 > baseline-characterization.txt   # maxDepth=8, 5 timing repeats
```

## The short version

- **`elreal` has no saturation depth.** Accuracy is linear in the knob and unbounded:
  every block buys another `k * log10(2)` decimal digits, for as long as you are
  willing to pay. Pick a depth from the accuracy you need, not from where the curve
  flattens, because it does not flatten.
- **`ereal` does saturate**, and its knob is compile-time. Its limbs are bare
  `double`s, so an expansion bottoms out at the host's `min_exponent` -- about 319
  digits, reached at `N = 12` for `sqrt`.
- **Cost varies ~90x between functions at the same depth.** `sqrt` and `exp` are
  cheap; `log` and the trigonometric functions are not.

## `elreal`: depth for a target accuracy

Accuracy is `depth * k * log10(2)` digits, where `k` is the host significand width.
Measured on `sqrt(2)`, double host:

| depth | 2 | 3 | 4 | 5 | 6 | 7 | 8 |
|---|---|---|---|---|---|---|---|
| digits | 32 | 49 | 66 | 83 | 98 | 115 | 131 |

That is 16.4 digits per block against a predicted `53 * log10(2) = 15.95`. So to
reach a target, `depth ~ target_digits / (k * log10(2))`:

| target | `double` (k=53) | `float` (k=24) | `bfloat16` (k=8) |
|---|---|---|---|
| ~16 digits (a host double) | 2 | 3 | 7 |
| ~30 digits | 2 | 4 | 12 |
| ~50 digits | 3 | 7 | 20 |
| ~100 digits | 7 | 13 | 40 |
| ~500 digits | 32 | 70 | 209 |

A narrower host is not less accurate, only less dense: `float` needs about 2.2x the
blocks of `double` for the same digits, and `bfloat16` about 6.6x.

## `elreal`: cost is function-dependent, not uniform

Per evaluation at depth 8 on a double host, all reaching ~130 digits:

| function | time | | function | time |
|---|---|---|---|---|
| `sqrt` | 4.1 ms | | `log` | 373.9 ms |
| `exp` | 11.2 ms | | `sin` | 371.7 ms |
| `tanh` | 11.8 ms | | `cos` | 371.6 ms |
| `sinh` / `cosh` | 15.7 ms | | `tan` | 374.3 ms |

A single global depth is therefore a poor fit for code that mixes them: the same knob
that costs 4 ms in `sqrt` costs 374 ms in `sin`. Set `precision()` per object, or
scope it with `elreal_precision_guard`, rather than raising the global default for the
benefit of one call site.

Reducing the `log`/trig constant is separate work -- it is the series evaluation, not
the precision machinery.

## The default

`kElrealDefaultPrecision` is **32 blocks** (~510 digits on a double host, ~231 on
float, ~77 on bfloat16). Because there is no saturation point, this is a policy choice
about how much headroom a caller gets before having to ask, not a measurement.

It governs the **class facade** only -- an `elreal`'s own `_depth`, used for boundary
operations (conversion, comparison, I/O) and for sizing facade arithmetic. The free
ZBCL math functions take their own `depth` arguments and are unaffected by it.

Cost is roughly linear in the knob. On the facade at `-O2`, double host:

| operation | at 8 blocks | at 32 blocks |
|---|---|---|
| `a / b` | 0.114 ms | 0.576 ms |
| `a * b` | 0.240 ms | 1.004 ms |
| `a + b` | 0.022 ms | 0.095 ms |
| `double(a)` | ~0 | ~0 |

Override per scope when the default is the wrong trade:

```cpp
{
    sw::universal::elreal_precision_guard guard(4);   // ~66 digits, much cheaper
    // ... work here uses depth 4 ...
}                                                     // restored on scope exit
```

## `ereal`: choosing `N`

`ereal<N>`'s knob is a compile-time limb count, so this is guidance for choosing the
template argument rather than a runtime setting. It genuinely saturates:

| function | saturates at | digits |
|---|---|---|
| `sqrt` | `N = 12` | 319 |
| `exp`, `log`, trig, hyperbolics | not within `N = 16` | 255-263 and still climbing |

The ceiling is the host's exponent range, not the limb count: an expansion of
`double`s cannot represent a term below `min_exponent`. Past `N = 12` for `sqrt`,
further limbs buy nothing.

## Which type for a given target

The tool reports this directly. At >= 30 digits, measured:

- `elreal` is cheaper for `sqrt`, `exp`, and the hyperbolics.
- `ereal` is cheaper for `log` and the trigonometric functions, by an order of
  magnitude -- which is the same `log`/trig constant noted above, seen from the other
  side.

## A note on reading the tool's summary

The per-function summary distinguishes:

```
saturates ~depth K (D digits)                  -- a plateau was observed at K
NO saturation through depth K (D digits, still climbing)
```

The second is not a failure to measure; for `elreal` it is the correct answer. An
earlier version reported the first in both cases, because it took "the first knob
within 95% of the best digits" and, for a series still climbing, the best is the last
row -- so it returned the operator's choice of `maxDepth` as though it were a property
of the type (#1177).
