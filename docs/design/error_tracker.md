# Error Tracking Design Sketch


1. Design Document

  ./docs/design/error-propagation-design.md - High-level comparison of strategies

2. Implementation Sketch

  ./docs/design/error_tracker_sketch.hpp - Concrete code sketch

## Key Design Decisions

### For cfloat (IEEE):

  - Use two_sum/two_prod for exact error computation
  - TrackedExact<float> gives perfect error tracking

### For posits:

  - Shadow computation with double/long double reference
  - TrackedShadow<posit<32,2>> computes exact reference in shadow type
  - Error = |shadow - value| after each operation

### For LNS (special case):

  - Multiplication is exact (log(a*b) = log(a) + log(b))
  - Only addition introduces error
  - Track additions and multiplications separately
  - Model cancellation amplification when a ≈ -b

### For areal (faithful floating-point with uncertainty bit):

  - Inherent uncertainty tracking via ubit (bit 0 of encoding)
  - When ubit=0: value is exact
  - When ubit=1: true value lies in open interval (v, next(v))
  - TrackedAreal<areal<32,8>> wraps with operation counting
  - Error bound = 0 if exact, otherwise width to next encoding

### For interval (classical interval arithmetic):

  - Rigorous mathematical bounds via directed rounding
  - [a,b] represents all x where a <= x <= b
  - TrackedInterval<double> provides containment guarantees
  - Error = interval width (enclosure of all possible values)

### For valid (posit-based intervals):

  - Uses two posit bounds (lb, ub) with open/closed indicators
  - Inherent uncertainty tracking like interval
  - Combines posit's dynamic range with interval rigor

## Trade-offs

  ┌─────────────┬──────────┬──────────┬──────────┬──────────┬──────────┬─────────────┐
  │  Strategy   │  cfloat  │  posit   │   LNS    │  areal   │ interval │    Cost     │
  ├─────────────┼──────────┼──────────┼──────────┼──────────┼──────────┼─────────────┤
  │ Exact       │ Perfect  │ N/A      │ N/A      │ N/A      │ N/A      │ Fast        │
  ├─────────────┼──────────┼──────────┼──────────┼──────────┼──────────┼─────────────┤
  │ Shadow      │ Good     │ Best     │ Good     │ N/A      │ N/A      │ 2x compute  │
  ├─────────────┼──────────┼──────────┼──────────┼──────────┼──────────┼─────────────┤
  │ Statistical │ Approx   │ Approx   │ Poor*    │ N/A      │ N/A      │ Fast        │
  ├─────────────┼──────────┼──────────┼──────────┼──────────┼──────────┼─────────────┤
  │ Bounded     │ Rigorous │ Rigorous │ Rigorous │ N/A      │ N/A      │ Pessimistic │
  ├─────────────┼──────────┼──────────┼──────────┼──────────┼──────────┼─────────────┤
  │ Inherent    │ N/A      │ N/A      │ N/A      │ Native   │ Native   │ Zero extra  │
  └─────────────┴──────────┴──────────┴──────────┴──────────┴──────────┴─────────────┘

  *Statistical is poor for LNS because it doesn't account for exact multiplications.

## Type Selection Guide

  ┌────────────────┬────────────────────┬─────────────────────────────────────────┐
  │ Number System  │ Recommended        │ Notes                                   │
  ├────────────────┼────────────────────┼─────────────────────────────────────────┤
  │ float/double   │ TrackedExact       │ two_sum/two_prod for perfect tracking   │
  │ posit          │ TrackedShadow      │ Shadow computation, error = |shadow-v|  │
  │ lns            │ TrackedLNS         │ Track adds only, mult is exact          │
  │ areal          │ TrackedAreal       │ Native ubit tracking, zero overhead     │
  │ interval       │ TrackedInterval    │ Native bounds, rigorous containment     │
  │ valid          │ (use directly)     │ Posit-based interval, inherent tracking │
  └────────────────┴────────────────────┴─────────────────────────────────────────┘

## Questions for Your Review

  1. Shadow type selection: Should posit<32,2> shadow to double, or to posit<64,3>?
  2. LNS error model: The cancellation factor 1/|1+ratio| is a first approximation. Do you have a more precise model?
  3. Error accumulation: Should we track absolute error, relative error, or both?
  4. Performance: Is 2x compute overhead acceptable for Shadow strategy, or should Statistical be the default for non-IEEE types?
  5. API: Should the interface be Tracked<T> with automatic strategy, or explicit like TrackedShadow<T>?
  6. Areal integration: Should TrackedAreal be a thin wrapper or provide additional analysis beyond the native ubit?
  7. Interval type: Should Universal add a standalone `interval<Real>` type, or is the existing `valid` sufficient?

