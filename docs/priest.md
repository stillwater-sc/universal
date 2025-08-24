# Douglas Priest multi-component adaptive precision arithmetic



Douglas Priest's multi-component arbitrary precision arithmetic approach models the approximation to the Reals through a set of doubles.
It is instructive to evaluate Priest's arbitrary precision aritmetic in how it handles infinite decimal expansions like 1/3.
Douglas Priest presented his approach handling infinite precision in a 1991 paper.
Algorithms for adaptive precision and how Priest's approach handles precision control were updated by Jonathan Shewchuk paper 
and extends Priest's work how the termination criteria work: 

Based on Douglas Priest's work and the adaptive precision arithmetic approach pioneered by him and extended by Jonathan Shewchuk, 
we can now provide a comprehensive answer to the question about handling infinite decimal expansions like 1.0/3.0.

## How Priest Handles Infinite Iterations in Arbitrary Precision Arithmetic

**The key insight is that Priest's approach is not truly "infinite precision" in the sense of computing infinite decimal expansions.** Instead, it uses **adaptive precision control** with specific termination criteria. Here's how it works:

### 1. **Adaptive Precision Rather Than Infinite Precision**

Priest's algorithms are adaptive in the sense that they do only as much work as necessary to guarantee a correct result. The system doesn't attempt to compute infinite decimal expansions but rather computes results to whatever precision is needed to meet a specified accuracy requirement.

### 2. **Error-Bound Driven Termination**

The approach uses a technique for adaptive precision arithmetic that can often speed these algorithms when one wishes to perform multiprecision calculations that do not always require exact arithmetic, but must satisfy some error bound. For operations like 1.0/3.0:

  - The system computes successive approximations with known error bounds

  - It terminates when the error bound is small enough for the intended use

  - A sequence of increasingly accurate approximations can be formed incrementally

### 3. **Practical Implementation Strategy**

When implementing 1.0/3.0 in Priest's system, you would:

1. **Specify precision requirements upfront** - either absolute error tolerance or number of significant digits needed

2. **Use iterative refinement** - start with hardware precision, then refine as needed

3. **Apply termination criteria** based on:

   - Desired relative error (e.g., accurate to 1 part in 10^n)

   - Absolute error bounds

   - Maximum number of terms/iterations

   - Use case requirements


### 4. **Multiple-Component Representation**

Each arbitrary precision value is expressed as an expansion e = e₀ + e₁ + ... + eₙ, where each eᵢ is called a component of e and is represented by a floating-point value. For 1/3, this means:

  - Representing the result as a sum of floating-point components

  - Each component captures part of the infinite decimal expansion

  - The expansion length is determined by precision requirements

### 5. **Practical Termination Example**

For 1.0/3.0 with a required precision of 50 decimal digits:

  - Compute components until the remaining error is less than 10^-50

  - The expansion might have 3-4 components in IEEE double precision

  - Each component is exactly representable in floating-point

### 6. **Application-Driven Precision**

The required degree of accuracy depends on their inputs. In your Universal Number Library implementation, you would provide:

  - User-specified precision parameters

  - Automatic precision adjustment based on intermediate results

  - Early termination when sufficient accuracy is achieved

## Recommendation for Your Implementation

For the Universal Number Library implementation:

1. **Provide precision control parameters** - allow users to specify required accuracy

2. **Implement lazy evaluation** - compute only as much precision as needed

3. **Use Priest's expansion representation** - store results as sums of floating-point components  

4. **Include automatic termination criteria** - based on error bounds and use-case requirements

5. **Support both absolute and relative error bounds** - giving users flexibility in precision control


This approach avoids the infinite iteration problem by making precision finite and adaptive, which is much more practical than attempting to compute truly infinite decimal expansions.

## Section 2

Looking at the progression from fixed-size expansions (dd, td, qd) to full adaptive Priest arithmetic, 
there is a clear architectural path that maximizes code reuse and provides a smooth upgrade path. 

## Overall Architecture Strategy

### 1. **Layered Component Hierarchy**

```cpp

// Base expansion operations (reusable across all types)

namespace universal::expansion {

    // Core algorithms that work on raw double arrays

    class ExpansionOps {
        static void two_sum(double a, double b, double& x, double& y);
        static void fast_two_sum(double a, double b, double& x, double& y);
        static void two_product(double a, double b, double& x, double& y);
        static void split(double a, double& hi, double& lo);

        // Variable-length operations
        static size_t grow_expansion(const double* e, size_t elen, double b, double* h);
        static size_t scale_expansion(const double* e, size_t elen, double b, double* h);
        static size_t fast_expansion_sum(const double* e, size_t elen, const double* f, size_t flen, double* h);
        static size_t compress(const double* e, size_t elen, double* h);
        static double estimate(const double* e, size_t elen);

    };
}



// Fixed-size wrappers

namespace universal {

    template<size_t N>
    class expansion.base; // Common interface for dd, td, qd

    using dd = expansion.base<2>;  // existing double-double
    using td = expansion.base<3>;  // new triple-double  
    using qd = expansion.base<4>;  // existing quad-double

    class priest;  // Full adaptive version

}

```

### 2. **Progressive Implementation Plan**


#### **Phase 1: Refactor Existing Code**

  - Extract common expansion operations from existing dd/qd into `ExpansionOps`

  - Create `expansion_base<N>` template that uses these operations

  - Migrate dd and qd to use the new base (should be drop-in compatible)


#### **Phase 2: Implement Triple-Double (td)**

```cpp

template<>
class expansion.base<3> {

private:

    double e.3];  // e\[0] smallest, e\[2] largest

public:

    // Standard arithmetic operations
    expansion.base operator+(const expansion_base& other) const;
    expansion.base operator*(double scalar) const;
    expansion.base operator*(const expansion_base& other) const;

    // Conversion and utility
    explicit operator double() const { return ExpansionOps::estimate(e, 3); }
    bool is.zero() const;
    int sign() const;

    // Access for algorithms that need raw components
    const double* components() const { return e; }
    size_t size() const { return 3; }

};

```



#### **Phase 3: Full Priest Implementation**

```cpp

class priest {

private:
    std::vector<double> expansion;  // Dynamic size
    mutable std::vector<double> workspace;  // For intermediate calculations

    // Adaptive precision control
    struct precision_config {
        double abs_tolerance = 0.0;
        double rel_tolerance = 1e-15;
        size_t max_terms = 100;
        bool adaptive = true;
    } config;

public:

    // Constructors with precision hints
    priest(double val, const precision.config& cfg = {});
    priest(const expansion_base<2>& dd, const precision_config& cfg = {});
    priest(const expansion_base<3>& td, const precision_config& cfg = {});
    priest(const expansion_base<4>& qd, const precision_config& cfg = {});

    // Adaptive arithmetic
    priest operator+(const priest. other) const;
    priest operator*(const priest. other) const;
    priest operator/(const priest. other) const;  // Iterative division

    // Precision management
    void set.precision(const precision_config& cfg);
    priest. compress();  // Remove unnecessary terms
    size_t num_terms() const { return expansion.size(); }

};

```

### 3. **Key Architectural Decisions**

#### **Shared Component Management**

```cpp

namespace universal::expansion {

    // Reusable algorithms that work on any expansion

    class ExpansionArithmetic {
    public:
        // These work on raw double arrays of any size
        static size_t add_expansions(const double* a, size_t alen,
                                     const double* b, size_t blen,
                                     double* result);

        static size_t multiply_expansion_by_expansion(
            const double* a, size_t alen,
            const double* b, size_t blen,
            double* result);

        static size_t divide_expansion_by_expansion(
            const double* a, size_t alen,
            const double* b, size_t blen, 
            double* result,
            const precision.config& cfg);
    };

}

```



#### **Seamless Upgrade Path**

```cpp

// Users can upgrade precision seamlessly
dd my.dd = 1.0/3.0;
td my.td = td(my_dd);      // Promote dd to td
qd my.qd = qd(my_td);      // Promote td to qd  
priest my.priest = priest(my_qd);  // Promote qd to full adaptive

// Or go directly

priest adaptive.third = priest(1.0) / priest(3.0);  // Full adaptive division

```

### 4. **Adaptive Precision Integration**

The `priest` class would provide the full adaptive behavior:

```cpp

class priest {

    // Division with adaptive precision
    priest operator/(const priest. other) const {
        if (config.adaptive) {
            // Start with estimate, refine as needed
            priest result = estimate.quotient(*this, other);

            while (!meets.precision_criteria(result, *this, other)) {
                result = refine.quotient(result, *this, other);
                if (result.num.terms() >= config.max_terms) break;
            }

            return result.compress();

        } else {
            // Fixed precision division
            return fixed.precision_divide(*this, other, config);
        }
    }

private:

    bool meets.precision_criteria(const priest& quotient, const priest. dividend, const priest. divisor) const {
        priest remainder = dividend - quotient * divisor;
        double abs_error = std::abs(double(remainder));
        double rel_error = abs_error / std::abs(double(dividend));

        return (abs_error <= config.abs_tolerance) || 
               (rel_error <= config.rel_tolerance);

    }

};

```



### 5. **Benefits of This Architecture**



1. **Incremental Development**: Build td first, then priest, reusing existing code

2. **Performance**: Fixed-size types (dd, td, qd) remain fast for when you don't need adaptivity

3. **Flexibility**: priest provides full adaptive precision when needed

4. **Compatibility**: Existing dd/qd code continues to work

5. **Upgrade Path**: Easy conversion between precision levels

6. **Code Reuse**: Core expansion algorithms shared across all types



### 6. **Usage Examples**

```cpp

// Fixed precision for speed
td fast.calc = td(1.0) / td(3.0);  // Fixed at 3 terms

// Adaptive precision for accuracy
priest precise.calc = priest(1.0) / priest(3.0);  // Adapts as needed

// Custom precision requirements
precision_config high_precision{
    .rel.tolerance = 1e-50,
    .max.terms = 20,
    .adaptive = true
};

priest very_precise = priest(1.0, high_precision) / priest(3.0, high_precision);

```

This architecture provides a natural progression from the existing dd/qd to td to full adaptive priest, with maximum code reuse and clean upgrade paths.
