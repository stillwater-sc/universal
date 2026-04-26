# Constexpr for the type


Got this observation from the mixed-precision-dsp library:

- CMakeLists.txt: Universal pin v4.6.9 -> v4.6.10 (brings in constexpr IEEE-754 posit ctors from PR #717 upstream).
- design_cic_compensator in decimation_chain.hpp: upgraded 4 constants from const to constexpr:

| Constant |              Before          |           After            |                                 Why it works now                                  |
|----------|------------------------------|----------------------------|-----------------------------------------------------------------------------------|
| zero     | const T zero{}               | constexpr T zero = T(0)    | T(0) uses int ctor (was constexpr pre-v4.6.10 too), avoids the non-user-provided default ctor issue  |
| one      | const T one = T(1)           | constexpr T one = T(1)     | int ctor                                                                      |
| pi_T     | const T pi_T = T(pi)         | constexpr T pi_T = T(pi)   | new in v4.6.10: posit(double) ctor is now constexpr                           |
| two_pi_T | const T two_pi_T = T(two_pi) | constexpr T two_pi_T = T(two_pi) | same                                                                    |

What still must stay const (with inline comments explaining why):

- half, tiny, a0, a1 -- use operator/, which is not yet constexpr for posit
- R, M, D, N_T -- converted from non-constexpr function parameters
- shift, n_T, k_T -- depend on runtime loop state

Is it customary for arithmetic user types to make the operators constexpr as well? What type of programs would depend on such a facility?

Yes, increasingly customary -- for any arithmetic type marketed as a drop-in replacement for native int/float/double. The native types are intrinsically
constexpr, so a custom type that isn't creates a syntactic asymmetry that breaks generic code.

## Prior art

|        Library        |                    Constexpr coverage                    |
|-----------------------|----------------------------------------------------------|
| std::chrono::duration | All arithmetic since C++14                               |
| std::complex<T>       | Most operators since C++20, full since C++26             |
| Boost.Multiprecision  | Wherever the limb arithmetic permits (intrinsic-guarded) |
| Boost.SafeNumerics    | All ops constexpr (entire safety machinery is constexpr) |
| Eigen 3.4+            | Many Matrix operators marked constexpr                   |
| GLM                   | Constexpr arithmetic throughout vec/mat                  |
| fmt, std::format      | Heavy constexpr to support compile-time format parsing   |

## Programs that benefit

1. Pre-computed coefficient tables -- the user's exact CIC compensator case. FIR/IIR filter taps, FFT twiddle factors, polynomial approximations (Chebyshev,
   Pade, Gauss-Legendre nodes/weights), Kaiser/Hamming windows. Without constexpr arithmetic, these have to be either hand-tabulated or built at startup.
2. constinit and embedded targets -- environments forbidding dynamic initialization. Posits used as flash-resident constants in MCU firmware, or kernels
   that must be ROM-able. Without constexpr ops, every "constant" gets dynamic-init code.
3. Compile-time numerical assertions -- static_assert(rel_error<posit<32,2>>(approx, exact) < eps);. The unit test runs in the compiler; failures are
   diagnostics, not runtime crashes. Particularly valuable for proving precision contracts in mixed-precision algorithms.
4. Template metaprogramming with numeric branching -- if constexpr (cond_number<Real>(matrix) > threshold) use_iterative; else use_direct;. The choice is
   baked at compile time per instantiation.
5. Auto-tuning and code generation -- HPC libraries that explore precision/blocking parameter spaces at compile time (e.g., kernel_for_posit<N,es> selected
   from a constexpr error model).
6. Static-init-order safety -- translation-unit-order-independent constants, a perennial C++ pitfall.
7. Constant folding at low optimization levels -- posit<32,2> two_pi = 2*pi folded to a single bit pattern even at -O0. Matters for debug builds and for
   guaranteeing performance on compilers/targets without aggressive optimizers.

## What it would take for Universal

The next sensible step (after this PR series) would be operator+, operator-, operator* constexpr for posit. The pattern is the same one you just shipped:

- Promote the constituent helpers (blocktriple, quire arithmetic operators) to constexpr.
- Use std::is_constant_evaluated() (or C++23 if consteval) to dispatch around any intrinsics.
- operator/ is harder -- Newton-Raphson reciprocal needs careful constexpr-clean iteration; long-division has the same blockbinary chain you already started.

A pragmatic order: addition/subtraction first (covers DSP additive accumulation), multiplication (covers most filter math), 
division last (rarely needed at compile time anyway -- you can usually pre-multiply by a reciprocal constant).

## Cost/value call

For Universal specifically -- given its positioning as a drop-in for HPC/DSP/embedded -- the answer is yes, worth doing, but staged. You've now crossed the
foundational threshold (constexpr literals + constexpr blockbinary chain), so each subsequent operator is incremental rather than architectural.

