# Error Propagation Tracker Design

## Problem Statement

For IEEE-754 floats, we can use two_sum/two_prod to compute **exact** rounding errors:
```cpp
// a + b = s + e, where e is the exact error
auto [s, e] = two_sum(a, b);
```

For posits and LNS, these lemmas don't apply:
- **Posits**: Variable-width exponent field makes error separation impossible
- **LNS**: Multiplication is exact (log addition), but addition error is non-linear

Additionally, some number systems **inherently track uncertainty**:
- **Areal**: Has an uncertainty bit (ubit) that indicates if value is exact or interval
- **Valid**: Uses posit-based intervals with open/closed bounds
- **Interval**: Classical interval arithmetic with rigorous containment

## Design Options

### Option A: Strategy Pattern with Type-Specific Implementations

```cpp
template<typename T>
struct ErrorModel {
    // Must be specialized per number system
    static double estimateAdditionError(T a, T b, T result);
    static double estimateMultiplicationError(T a, T b, T result);
};

// IEEE floats: exact via two_sum/two_prod
template<>
struct ErrorModel<float> {
    static double estimateAdditionError(float a, float b, float result) {
        auto [s, e] = two_sum(a, b);
        return std::abs(static_cast<double>(e));
    }
};

// Posits: use ULP-based bounds
template<unsigned nbits, unsigned es>
struct ErrorModel<posit<nbits, es>> {
    static double estimateAdditionError(posit<nbits,es> a, posit<nbits,es> b,
                                         posit<nbits,es> result) {
        // Posit error is bounded by 0.5 ULP of the result
        double ulp = ulp_value(result);
        return 0.5 * ulp;  // Worst-case rounding error
    }
};

// LNS: error depends on operand ratio for addition
template<unsigned nbits, unsigned rbits>
struct ErrorModel<lns<nbits, rbits>> {
    static double estimateAdditionError(lns<nbits,rbits> a, lns<nbits,rbits> b,
                                         lns<nbits,rbits> result) {
        // LNS addition error depends on |a/b| ratio
        // When a ≈ -b (cancellation), error is large
        // Error model: based on log-domain quantization
        double ratio = std::abs(double(a) / double(b));
        double base_ulp = ulp_value(result);
        // Cancellation amplifies error
        if (ratio > 0.9 && ratio < 1.1) {
            return base_ulp * (1.0 / std::abs(1.0 - ratio));  // Amplified
        }
        return base_ulp;
    }
};
```

**Pros**: Type-specific accuracy, uses exact methods when available
**Cons**: Requires specialization for each type, complex to maintain

---

### Option B: Shadow Computation (Universal but Expensive)

Compute everything twice: once in the target type, once in high precision:

```cpp
template<typename T, typename Shadow = double>
class ShadowedValue {
    T value_;
    Shadow shadow_;  // High-precision shadow

public:
    ShadowedValue(T v) : value_(v), shadow_(static_cast<Shadow>(v)) {}

    ShadowedValue operator+(const ShadowedValue& rhs) const {
        T result = value_ + rhs.value_;
        Shadow exact = shadow_ + rhs.shadow_;
        // Error is difference between shadow and result
        return ShadowedValue(result, exact);
    }

    double error() const {
        return std::abs(static_cast<double>(shadow_) - static_cast<double>(value_));
    }

    double relative_error() const {
        if (shadow_ == Shadow(0)) return 0.0;
        return error() / std::abs(static_cast<double>(shadow_));
    }
};
```

**Pros**: Works for ANY number system, gives actual error not just bounds
**Cons**: 2x computation, requires convertibility to Shadow type

---

### Option C: Interval Arithmetic (Rigorous Bounds)

Track [lower, upper] bounds instead of point values:

```cpp
template<typename T>
class Interval {
    T lower_, upper_;

public:
    Interval(T v) : lower_(v), upper_(v) {}
    Interval(T lo, T hi) : lower_(lo), upper_(hi) {}

    Interval operator+(const Interval& rhs) const {
        // Addition: [a,b] + [c,d] = [a+c, b+d] with rounding
        T lo = lower_ + rhs.lower_;  // Round toward -inf
        T hi = upper_ + rhs.upper_;  // Round toward +inf
        return Interval(nextafter(lo, -INFINITY), nextafter(hi, INFINITY));
    }

    Interval operator*(const Interval& rhs) const {
        // Multiplication: must consider all combinations
        T a = lower_ * rhs.lower_;
        T b = lower_ * rhs.upper_;
        T c = upper_ * rhs.lower_;
        T d = upper_ * rhs.upper_;
        return Interval(std::min({a,b,c,d}), std::max({a,b,c,d}));
    }

    double width() const { return double(upper_) - double(lower_); }
    double relative_width() const {
        double mid = (double(upper_) + double(lower_)) / 2.0;
        return (mid != 0) ? width() / std::abs(mid) : 0.0;
    }
};
```

**Pros**: Rigorous bounds, works for any type, mathematically sound
**Cons**: Intervals widen (pessimistic), no exact error, requires directed rounding

---

### Option D: Inherent Uncertainty Tracking (Areal, Valid, Interval)

Some number systems track uncertainty at the type level:

#### Areal (Faithful Floating-Point with Uncertainty Bit)

```cpp
// Areal encoding: [sign | exponent | fraction | ubit]
// When ubit=0: value is exact
// When ubit=1: true value lies in (v, next(v))

template<typename ArealType>
class TrackedAreal {
    ArealType value_;
    uint64_t op_count_;

public:
    bool is_exact() const { return !value_.at(0); }  // Check ubit

    double error_bound() const {
        if (is_exact()) return 0.0;
        ArealType next_val = value_;
        ++next_val;  // Move to next encoding
        return std::abs(double(next_val) - double(value_));
    }

    TrackedAreal operator+(const TrackedAreal& rhs) const {
        // Areal handles uncertainty propagation internally via ubit
        return TrackedAreal(value_ + rhs.value_);
    }
};
```

**Pros**: Zero-overhead uncertainty tracking, built into the type
**Cons**: Only tracks to next encoding, not exact error

#### Classical Interval Arithmetic

```cpp
template<typename Real>
class interval {
    Real lo_, hi_;

public:
    interval operator+(const interval& rhs) const {
        // [a,b] + [c,d] = [a+c, b+d] with directed rounding
        return interval(round_down(lo_ + rhs.lo_),
                        round_up(hi_ + rhs.hi_));
    }

    double width() const { return hi_ - lo_; }
    bool contains(Real v) const { return lo_ <= v && v <= hi_; }
};
```

**Pros**: Rigorous mathematical guarantees, containment proof
**Cons**: Intervals widen over long computations (dependency problem)

---

### Option E: Hybrid Approach (Recommended)

Combine the best of each approach based on type capabilities:

```cpp
enum class ErrorStrategy {
    Exact,       // two_sum/two_prod (IEEE floats only)
    Shadow,      // High-precision shadow (universal)
    Bounded,     // Interval arithmetic (universal, rigorous)
    Statistical, // ULP-based model (fast, approximate)
    Inherent     // Type natively tracks uncertainty (areal, valid, interval)
};

// Type traits to detect capabilities
template<typename T>
struct error_tracking_traits {
    static constexpr bool has_exact_errors = false;
    static constexpr bool has_directed_rounding = false;
    static constexpr bool exact_multiplication = false;
    static constexpr bool tracks_uncertainty = false;  // Type natively tracks error
    static constexpr bool is_interval_type = false;
    static constexpr ErrorStrategy default_strategy = ErrorStrategy::Shadow;
};

template<>
struct error_tracking_traits<float> {
    static constexpr bool has_exact_errors = true;
    static constexpr bool has_directed_rounding = true;
    static constexpr bool exact_multiplication = false;
    static constexpr bool tracks_uncertainty = false;
    static constexpr bool is_interval_type = false;
    static constexpr ErrorStrategy default_strategy = ErrorStrategy::Exact;
};

template<>
struct error_tracking_traits<double> {
    static constexpr bool has_exact_errors = true;
    static constexpr bool has_directed_rounding = true;
    static constexpr bool exact_multiplication = false;
    static constexpr bool tracks_uncertainty = false;
    static constexpr bool is_interval_type = false;
    static constexpr ErrorStrategy default_strategy = ErrorStrategy::Exact;
};

template<unsigned nbits, unsigned es>
struct error_tracking_traits<posit<nbits, es>> {
    static constexpr bool has_exact_errors = false;
    static constexpr bool has_directed_rounding = false;
    static constexpr bool exact_multiplication = false;
    static constexpr bool tracks_uncertainty = false;
    static constexpr bool is_interval_type = false;
    static constexpr ErrorStrategy default_strategy = ErrorStrategy::Shadow;
};

template<unsigned nbits, unsigned rbits>
struct error_tracking_traits<lns<nbits, rbits>> {
    static constexpr bool has_exact_errors = false;
    static constexpr bool has_directed_rounding = false;
    static constexpr bool exact_multiplication = true;   // KEY: Mult is exact in LNS!
    static constexpr bool tracks_uncertainty = false;
    static constexpr bool is_interval_type = false;
    static constexpr ErrorStrategy default_strategy = ErrorStrategy::Shadow;
};

// Areal: faithful floating-point with uncertainty bit
template<unsigned nbits, unsigned es, typename bt>
struct error_tracking_traits<areal<nbits, es, bt>> {
    static constexpr bool has_exact_errors = false;
    static constexpr bool has_directed_rounding = false;
    static constexpr bool exact_multiplication = false;
    static constexpr bool tracks_uncertainty = true;   // Built-in uncertainty bit!
    static constexpr bool is_interval_type = true;
    static constexpr ErrorStrategy default_strategy = ErrorStrategy::Inherent;
};

// Valid: posit-based interval arithmetic
template<unsigned nbits, unsigned es>
struct error_tracking_traits<valid<nbits, es>> {
    static constexpr bool has_exact_errors = false;
    static constexpr bool has_directed_rounding = false;
    static constexpr bool exact_multiplication = false;
    static constexpr bool tracks_uncertainty = true;   // Open/closed bounds
    static constexpr bool is_interval_type = true;
    static constexpr ErrorStrategy default_strategy = ErrorStrategy::Inherent;
};

// Classical interval arithmetic
template<typename Real>
struct error_tracking_traits<interval<Real>> {
    static constexpr bool has_exact_errors = false;
    static constexpr bool has_directed_rounding = true;  // Uses directed rounding
    static constexpr bool exact_multiplication = false;
    static constexpr bool tracks_uncertainty = true;
    static constexpr bool is_interval_type = true;
    static constexpr ErrorStrategy default_strategy = ErrorStrategy::Inherent;
};
```

---

## Recommended Design: Tracked<T>

A unified wrapper that selects the best strategy automatically:

```cpp
template<typename T,
         ErrorStrategy Strategy = error_tracking_traits<T>::default_strategy>
class Tracked {
    T value_;

    // Strategy-specific state
    struct ExactState { double cumulative_error; };
    struct ShadowState { long double shadow; };
    struct BoundedState { T lower, upper; };
    struct StatState { double estimated_error; uint64_t op_count; };

    using State = std::conditional_t<Strategy == ErrorStrategy::Exact, ExactState,
                  std::conditional_t<Strategy == ErrorStrategy::Shadow, ShadowState,
                  std::conditional_t<Strategy == ErrorStrategy::Bounded, BoundedState,
                  StatState>>>;
    State state_;

public:
    // ... implementation based on Strategy
};
```

## LNS-Specific Considerations

LNS is unique because:
1. **Multiplication is exact** (addition in log domain)
2. **Addition is the error-prone operation** (requires log(1 + e^x) computation)
3. **Catastrophic cancellation** when a ≈ -b

For LNS, the error model should account for:
```cpp
// LNS addition: log(a + b) = log(a) + log(1 + b/a)
// When b/a ≈ -1, we have cancellation
// Error amplification factor: 1 / |1 + b/a|

template<unsigned nbits, unsigned rbits>
double lns_addition_error_factor(lns<nbits,rbits> a, lns<nbits,rbits> b) {
    double ratio = double(b) / double(a);
    if (std::abs(1.0 + ratio) < 1e-10) {
        return 1e10;  // Severe cancellation
    }
    return 1.0 / std::abs(1.0 + ratio);
}
```

## API Design

```cpp
// Simple usage
Tracked<posit<32,2>> x = 1.0;
Tracked<posit<32,2>> y = 1e-10;
auto z = x + y;

std::cout << "Value: " << z.value() << "\n";
std::cout << "Estimated error: " << z.error() << "\n";
std::cout << "Relative error: " << z.relative_error() << "\n";
std::cout << "Valid bits: " << z.valid_bits() << "\n";

// Explicit strategy selection
Tracked<float, ErrorStrategy::Exact> a = 1.0f;      // Use two_sum
Tracked<float, ErrorStrategy::Shadow> b = 1.0f;     // Use shadow
Tracked<float, ErrorStrategy::Bounded> c = 1.0f;    // Use intervals
```

## Trade-offs Summary

| Strategy | Accuracy | Performance | Universality | Memory | Best For |
|----------|----------|-------------|--------------|--------|----------|
| Exact | Perfect | Fast | IEEE only | Low | float/double |
| Shadow | High | 2x slower | Universal | 2x | posit |
| Bounded | Rigorous | Moderate | Universal | 2x | Certification |
| Statistical | Approximate | Fast | Universal | Low | High-perf |
| Inherent | Type-dependent | Native | Inherent types | 1x | areal/valid/interval |

### Type-Specific Recommendations

| Number System | Recommended Strategy | Notes |
|---------------|---------------------|-------|
| float/double | Exact | two_sum/two_prod available |
| posit | Shadow | No error-free transforms |
| lns | LNS-specific | Mult is exact, track add errors only |
| areal | Inherent | Uses built-in uncertainty bit |
| valid | Inherent | Posit-based intervals with open/closed |
| interval | Inherent | Classical IA with directed rounding |

## Recommendation

1. **Use Exact** for float/double when available (perfect error tracking)
2. **Default to Shadow** for posits (most accurate for tapered precision)
3. **Use LNS-specific** for LNS (exploit exact multiplication)
4. **Use Inherent** for areal/valid/interval (they track uncertainty natively)
5. **Offer Bounded** for applications needing rigorous guarantees
6. **Use Statistical** for high-performance scenarios where bounds are acceptable

## Design Philosophy

The key insight is that different number systems have fundamentally different error characteristics:

- **IEEE floats**: Uniform precision, error-free transforms exist
- **Posits**: Tapered precision, no clean error separation
- **LNS**: Multiplication is exact, only addition introduces error
- **Areal**: Built-in uncertainty tracking via ubit
- **Interval**: Rigorous bounds via directed rounding

A unified error tracker must respect these differences while providing a consistent API.
