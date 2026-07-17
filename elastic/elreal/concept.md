# elreal facade: exact lazy real


## Inventory: the gap

|            |                               ereal (eager Priest — the model)                                |      elreal (lazy LFPERA — today)      |
|--------------|-----------------------------------------------------------------------------------------------|----------------------------------------|
| Facade class | template<unsigned maxLimbs> class ereal wrapping std::vector<double> _limb | none — only free functions over ZBCL<FpType> |
| fwd        | class ereal; + abs/fabs                                                                       | only struct block; / class ZBCL;       |
|            | ereal_impl.hpp (48 KB): native ctors, operator=, explicit operator double/float/long double,  |                                        |
| impl       | += -= *= /= (eager expansion_ops), unary -, operator[], free + - * / and == < … (ereal×ereal, | —                                      |
|            |  ×double, double×), abs/fabs                                                                  |                                        |
| support    | numeric_limits.hpp, attributes.hpp, manipulators.hpp, mathlib.hpp, traits/ereal_traits.hpp    | none of these                          |
|            |                                                                                               | lazy/online: add, mul_online,          |
| arithmetic | eager, recomputes precision each call                                                         | div_online, infsum over ZBCL; math     |
|            |                                                                                               | suite f(ZBCL, depth)                   |
| value rep  | std::vector<double> (bounded, eager)                                                          | ZBCL<FpType> (shared_ptr, memoized,    |
│            |                                                                                               │ unbounded, pull-driven)                |

Key structural fact: ereal holds a std::vector → it is elastic, not trivially copyable. elreal holding a ZBCL (shared_ptr) is the same — so the "trivial
type" rule (#925, for hardware-shareable static types like block) does not apply; elreal joins the elastic family (einteger/edecimal/erational/ereal). That
resolves the apparent tension up front.

## Core design: class elreal<FpType = double>

Mirror ereal's facade, but the wrapped value is the lazy `ZBCL<FpType>`, and the operators stay lazy:

```cpp
template<typename FpType = double>
  class elreal {
      ZBCL<FpType> _value;                       // the lazy state machine (memoized)
      std::size_t  _depth = kDefaultDepth;       // default pull depth for boundary ops
  public:
      // --- standard plug-in facade (mirrors ereal) ---
      elreal();                                  // 0  (empty ZBCL)
      elreal(const elreal&) = default; ...       // copy/move = default (shared_ptr: cheap)
      elreal(int/long/float/double/long double);  // from_native
      elreal(const std::string&);                // parse
      elreal(SpecificValue);                     // qnan/inf/zero/...
      elreal& operator=(native);                 // convert
      explicit operator double()  const;         // to_double_approx(_value, _depth)
      explicit operator float()/long double() const;

      elreal  operator-() const;                 // negate (lazy)
      elreal& operator+=(const elreal&);         // _value = add(_value, rhs._value)   (lazy)
      elreal& operator-=, operator*=(mul_online), operator/=(div_online);

      // --- lazy API / state-machine extension (the elreal-specific surface) ---
      std::vector<block<FpType>> limbs(std::size_t n) const;   // pull n limbs (reuses memoized work)
      template<class T = double> T approx(std::size_t depth) const;
      elreal& refine(std::size_t depth);          // advance the pull frontier *incrementally*
      std::size_t precision() const;  elreal& precision(std::size_t);
      const ZBCL<FpType>& stream() const;         // raw state-machine handle (advanced)
  };
  // free + - * / (elreal², ×native, native×), == != < <= > >=, abs/fabs
```

Lazy semantics (the whole point): c = a + b stores the unforced add(a._value, b._value). Nothing computes until a boundary op forces it: double(c), c == d,
std::cout << c, or an explicit c.approx(depth). Because ZBCL memoizes, asking for depth d then d+1 reuses prior work — the McCleeary edge over ereal,
which recomputes the whole expansion every time. refine() exposes exactly that incremental advance.

Files to create (mirroring ereal/)

1. elreal_impl.hpp — class elreal<FpType> + free operators.
2. numeric_limits.hpp, attributes.hpp (sign/scale/range), manipulators.hpp (type_tag/to_binary/to_hex/components/color_print), mathlib.hpp (elreal-level
exp/sin/sqrt/... wrapping the ZBCL versions).
3. traits/elreal_traits.hpp — is_elreal / enable_if_elreal.
4. Edit elreal_fwd.hpp (add class elreal; + math fwd), elreal.hpp (include the new headers), add aliases (elreal64=elreal<double>, elreal32=elreal<float>).
5. static/elreal/api/api.cpp — the canonical "how to use it" test (plug-in template-kernel usage), plus logic/arithmetic/conversion suites.

Phased implementation order

1. Scaffold + facade: fwd/impl/traits, ctors, conversions, the four operators delegating to the lazy ops; one api.cpp proving plug-in use in a templated
kernel. (non-breaking — all existing ZBCL machinery stays)
2. Lazy extension: limbs/approx/refine/precision/stream + a laziness/memoization test (pull d, then d+1, assert 1..d not recomputed).
3. Boundary ops: conversions, comparisons (depth-bounded), I/O, numeric_limits, manipulators, attributes.
4. Math facade: mathlib.hpp elreal overloads; migrate constants to elreal returns.

Decisions I need from you before scaffolding

How should the default precision (depth for conversions/comparisons/printing) be carried?

❯ 1. Runtime member + scoped override (rec.)
     elreal<FpType> with a per-object _depth member, settable via .precision(n), plus a scoped/thread-local default (like iostream precision or mpfr). Most
     natural for a lazy type whose precision is inherently dynamic; template stays clean.
  2. Template param: elreal<FpType, Depth>
     Bake default depth into the type, like ereal<maxLimbs>. Type-safe and matches the ereal precedent, but rigid -- two precisions are different types, and
     incremental refinement wants runtime control.
  3. Global/static default only
     A single library-wide default depth (constexpr or static). Simplest, but no per-object or scoped control -- weakest fit for a lazy incremental type.
  4. Type something.


What semantics should the arithmetic operators have?

❯ 1. Fully lazy, force at boundaries (rec.)
     operator+/*// store unforced ZBCLs; evaluation happens only on conversion/compare/print/explicit eval. Maximizes the incremental-reuse advantage (the
     reason elreal exists). Cost: a chain builds a lazy tower until forced.
  2. Lazy storage, auto-eval to default depth
     Operators return lazy results but each assignment materializes to the default depth (then keeps the stream for refinement). More ereal-like/predictable
     cost, but partly defeats incremental reuse.
  3. Type something.


Equality/ordering for two lazy exact reals is only semi-decidable. How should ==/< behave?

❯ 1. Depth-bounded compare (rec.)
     ==/< materialize both operands to the default depth and compare, documented as precision-bounded (cannot prove exact equality of two irrationals).
     Matches plug-in expectations; add an exact-when-terminating fast path.
  2. Sign-of-difference compare
     Compute a-b lazily and pull limbs until a nonzero limb decides the sign (true exact ordering when they differ); equality still needs a depth bound. More
     'exact-real' correct, more complex, can be slow near equal values.
  3. Type something.

Review your answers

 ● How should the default precision (depth for conversions/comparisons/printing) be carried?
   → Runtime member + scoped override (rec.)
 ● What semantics should the arithmetic operators have?
   → Fully lazy, force at boundaries (rec.)
 ● Equality/ordering for two lazy exact reals is only semi-decidable. How should ==/< behave?
   → Depth-bounded compare (rec.)

